# Optional - Specular Map (You are not required to do this)

# Resources to Help

- https://learnopengl.com/Lighting/Lighting-maps

# Description

Recall that we have introduced previously diffuse texture maps -- that is textures that contain the colors that will be interpolated across a triangle. We can further use textures to store other information, such as the different material properties. The advantage of this is that we get to store at a per-pixel level some data -- whether that be the shininess, roughness, or some other property. For this (**optional**) portion of the assignment, you can enhance your texture loader to also handle a **specular map** for the shininess.

## Material Files

Most material files if they support other types of maps, will support a specular map. The `map_Ks` shown below highlights the specific map for the specular texture data.

```
# Blender MTL File: 'None'
# Material Count: 1

newmtl blinn1SG.001
Ns 0.000000
Ka 0.100000 0.100000 0.100000
Kd 0.000000 0.000000 0.000000
Ks 0.000000 0.000000 0.000000
Ke 0.000000 0.000000 0.000000
Ni 1.500000
d 1.000000
illum 2
map_Kd house_diffuse.ppm
map_Bump house_normal.ppm
map_Ks house_spec.ppm
```

## Multiple texture slots

In this section I'll show some pseudocode of the basic idea. Recall that textures are 'uniforms' and that you can bind to more than one texture object at a time. The following binds the diffuse texture map object to whatever texture is bound to slot 0.

```
/// Some .cpp file
myShader.setUniformInt("diffuseTextureMap", 0);
...
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, diffuseMap);
```

Here we repeat the same, this time binding a second texture to texture slot 1 (GL_TEXTURE1 is the active texture, and sometimes folks also access this as GL_TEXTURE0+1 since it is purely an enum).

```
/// Some .cpp file
myShader.setUniformInt("specularTextureMap", 1);
...
glActiveTexture(GL_TEXTURE1);
glBindTexture(GL_TEXTURE_2D, specularMap);  
```

Then within your shader, you will access the texture as follows:

```
/// Fragment Shader code
vec3 diffuse	= light.diffuse * vec3(texture(diffuseTextureMap, vTexCoords));
vec3 specular   = light.specular * vec3(texture(specularTextureMap, vTexCoords));
```

# Deliverable

Enhance your 3D textured loader to also load and handle a specular map.

## Examples

From the learnopengl page, here is again an example of the diffuse and specular maps.

<img src="./media/container2.png">

<img src="./media/container2_specular.png">

