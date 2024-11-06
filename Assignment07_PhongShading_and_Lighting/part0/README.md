## Part 0 - Abstraction

We have reached a point where I think it becomes necessary for the debuggability of our programs to split things into different files.

For the purpose of beginning to learn computer graphics, largely it has been useful to have everything in one file for quickly prototyping as we learn.

### Task 1

- You will find a code sample here that may be useful: [/7/stl_light](https://github.com/MikeShah/ComputerGraphicsCode/tree/master/7/stl_light)
- Here is a supporting video: https://youtu.be/ZU9wcDPc6kw

Here are a few notes regarding the abstraction and things to think about.

- The following code example demonstrates how to load a .stl model 
    - The STL formats differs a bit from the .obj file format, in the sense that it is just a list of 'triangles' (a.k.a. triangle soup).
    - Thus, the STL format simply uses glDrawArrays for rendering (i.e. there is no element buffer object)
- The code also introduces some abstraction for handling 'lights' in a separate file.
    - You'll observe that in order to render a 'light' (which is represented as a cube) there is a separate light and vertex fragment shader.
    - This is a small sample of the power of the programmable pipeline -- in that we can juse change 'glProgramUse(handle)' to setup a new programmable pipeline for any subsequent glDraw* calls.
- globals.hpp
    - Unfortunately we need some global mutable state in our graphics application. For now, the easiest thing to do is have a header file (and corresponding globals.cpp file) holding the state.
        - One thing we can do to improve globals (when we must have them), is to wrap them all into a single struct.
        - In my mind, that at least makes it more clear when you have global mutable state
            - As an aside -- if later on we added multiple threads to our application, it makes it somewhat trivial to add a monitor lock to ensure mutual exclusion to global variables.
- util.hpp
    - For now (Fall 2023) this is a set of useful functions. In practice, we might also want a 'shader' class and a 'shaderManager' such that we do not duplicate shaders.
        - I may add such functionality later on -- other useful member functions may be to 'set' and 'retrieve' uniform variables set in a pipeline.
- For the purpose of this class, I want to avoid over abstracting as much as possible. There may be a time where we want to utilize inheritance, entity-component systems, and interfaces, but that time is not quite yet -- keep your code debuggable.



## Task 2

Note: Here are some sample shaders -- **these are not** used in the sample code, nor necessarily what you will use in the assignment. That said, it may be useful to just look at this shader pipeline below to get an idea of working with point lights, creating structs, and functions.

### Vertex Shader in a glance

This is what you are suppose to implement. There are many new pieces to study. Study it.

In brief:
1. Notice that we are passing in a new set of data with the different 'layouts'.
2. We are sending in more 'uniforms' for our model, view, and project matrix.
3. We have several 'out' variables that will be passed to the fragment shader.
	- Consider why it might be important to update the 'fragment position' for instance.

```glsl
// ==================================================================
#version 410 core
// Read in our attributes stored from our vertex buffer object
// We explicitly state which is the vertex information
// (The first 3 floats are positional data, we are putting in our vector)
layout(location=0)in vec3 position; 
layout(location=1)in vec3 normals; // Our second attribute - normals.
layout(location=2)in vec2 texCoord; // Our third attribute - texture coordinates.
layout(location=3)in vec3 tangents; // Our third attribute - texture coordinates.
layout(location=4)in vec3 bitangents; // Our third attribute - texture coordinates.

// If we are applying our camera, then we need to add some uniforms.
// Note that the syntax nicely matches glm's mat4!
//
// For our objects, we can now have model space which are the objects position
// We also have a camera which is the 'view space' now
// And finally the 'projection' which will transform our vertices into our chosen projection (i.e. for us, a perspective view).
uniform mat4 model; // Object space
uniform mat4 view; // View space
uniform mat4 projection; // Projection space

// Export our normal data, and read it into our frag shader
out vec3 myNormal;
// Export our Fragment Position computed in world space
out vec3 FragPos;
// Since we now have texture coordinates we can now use this as well
out vec2 v_texCoord;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);

    myNormal = normals;
    // Transform normal into world space
    FragPos = vec3(model* vec4(position,1.0f));

    // Store the texture coordinatests which we will output to
    // the next stage in the graphics pipeline.
    v_texCoord = texCoord;
}
// ==================================================================

```

### Fragment Shader in a glance

This is what you are suppose to implement. There are many new pieces to study. Study it.

In brief:
1. You will observe we still only ever output one value, FragColor, from our fragment shader
2. We can create `struct`'s in our shaders to help organize the scene.
	- We can create as many lights as we like and pass that 'lighting information' in an array.
	- Notice the uniform for 'pointLights'. 
		- We set the values in [./src/Object.cpp](./src/Object.cpp)
		- We then use those values in our shader to light our triangles.

```glsl
// ==================================================================
#version 410 core

// The final output color of each 'fragment' from our fragment shader.
out vec4 FragColor;

// Our light source data structure
struct PointLight{
    vec3 lightColor;
    vec3 lightPos;
    float ambientIntensity;

    float specularStrength;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// A uniform for '1' point light.
// We communicate the values of this point light from our CPU code
uniform PointLight pointLights[1];

// Used for our specular highlights
uniform mat4 view;

// Import our normal data
in vec3 myNormal;
// Import our texture coordinates from vertex shader
in vec2 v_texCoord;
// Import the fragment position
in vec3 FragPos;

// If we have texture coordinates, they are stored in this sampler.
uniform sampler2D u_DiffuseMap; 

void main()
{
    // Compute the normal direction
    vec3 norm = normalize(myNormal);
    
    // Store our final texture color
    vec3 diffuseColor;
    diffuseColor = texture(u_DiffuseMap, v_texCoord).rgb;

    // (1) Compute ambient light
    vec3 ambient = pointLights[0].ambientIntensity * pointLights[0].lightColor;

    // (2) Compute diffuse light
    // From our lights position and the fragment, we can get
    // a vector indicating direction
    // Note it is always good to 'normalize' values.
    vec3 lightDir = normalize(pointLights[0].lightPos - FragPos);
    // Now we can compute the diffuse light impact
    float diffImpact = max(dot(norm, lightDir), 0.0);
    vec3 diffuseLight = diffImpact * pointLights[0].lightColor;

    // (3) Compute Specular lighting
    vec3 viewPos = vec3(0.0,0.0,0.0);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = pointLights[0].specularStrength * spec * pointLights[0].lightColor;

    // Calculate Attenuation here
    // distance and lighting... 

    // Our final color is now based on the texture.
    // That is set by the diffuseColor
    vec3 Lighting = diffuseLight + ambient + specular;

    // Final color + "how dark or light to make fragment"
    if(gl_FrontFacing){
        FragColor = vec4(diffuseColor * Lighting,1.0);
    }else{
        // Additionally color the back side the same color
        // This is not strictly necessary, but sometimes you may want to light the inside different than the outside to help you debug if you are within some shape.
         FragColor = vec4(diffuseColor * Lighting,1.0);
    }
}

```

# F.A.Q. (Instructor Anticipated Questions)

* Q: Can I use the provided code?
  * A: You are welcome to use (with citation) the code provided. Mind you, remember you are working with .obj files, and that the provided code does not implement the phong illumination model.


# Found a bug?

If you found a mistake (big or small, including spelling mistakes) in this lab, kindly send me an e-mail. It is not seen as nitpicky, but appreciated! (Or rather, future generations of students will appreciate it!)

- Fun fact: The famous computer scientist Donald Knuth would pay folks one $2.56 for errors in his published works. [[source](https://en.wikipedia.org/wiki/Knuth_reward_check)]
- Unfortunately, there is no monetary reward in this course :)
