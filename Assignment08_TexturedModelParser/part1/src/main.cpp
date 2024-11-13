/* Compilation on Linux:
 g++ -std=c++17 ./src/*.cpp -o prog -I ./include/ -I./../common/thirdparty/ -lSDL2 -ldl
*/

// Third Party Libraries
#define GLM_ENABLE_EXPERIMENTAL
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

// C++ Standard Template Library (STL)
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

// Our libraries
#include "Camera.hpp"
#include "OBJMesh.hpp"

// vvvvvvvvvvvvvvvvvvvvvvvvvv Globals vvvvvvvvvvvvvvvvvvvvvvvvvv
// Globals generally are prefixed with 'g' in this application.

// Screen Dimensions
int gScreenWidth                        = 640;
int gScreenHeight                       = 480;
SDL_Window* gGraphicsApplicationWindow  = nullptr;
SDL_GLContext gOpenGLContext            = nullptr;

// Main loop flag
bool gQuit = false; // If this is quit = 'true' then the program terminates.

// shader
// The following stores the a unique id for the graphics pipeline
// program object that will be used for our OpenGL draw calls.
GLuint gGraphicsPipelineShaderProgram   = 0;

// OpenGL Objects
GLuint gVertexArrayObjectFloor= 0;
GLuint gVertexBufferObjectFloor = 0;

// Camera
Camera gCamera;

// Draw wireframe mode
GLenum gPolygonMode = GL_FILL;
OBJMesh gMesh;
bool gRenderModel = true;  // Controls whether to render the model
size_t gFloorResolution = 10;
size_t gFloorTriangles  = 0;

GLuint gVertexArrayObjectModel = 0;
GLuint gVertexBufferObjectModel = 0;
size_t gModelTriangles = 0;

// Shading mode: 0 - normals, 1 - Phong lighting
int g_shadingMode = 1;

GLuint gVertexArrayObjectLight = 0;
GLuint gVertexBufferObjectLight = 0;
size_t gLightBoxVertices = 0;
GLuint gDiffuseTexture = 0;

// ^^^^^^^^^^^^^^^^^^^^^^^^ Globals ^^^^^^^^^^^^^^^^^^^^^^^^^^^


// vvvvvvvvvvvvvvvvvvv Error Handling Routines vvvvvvvvvvvvvvv
static void GLClearAllErrors(){
    while(glGetError() != GL_NO_ERROR){
    }
}

// Returns true if we have an error
static bool GLCheckErrorStatus(const char* function, int line){
    while(GLenum error = glGetError()){
        std::cout << "OpenGL Error:" << error
                  << "\tLine: " << line
                  << "\tfunction: " << function << std::endl;
        return true;
    }
    return false;
}

#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x,__LINE__);
// ^^^^^^^^^^^^^^^^^^^ Error Handling Routines ^^^^^^^^^^^^^^^



/**
* LoadShaderAsString takes a filepath as an argument and will read line by line a file and return a string that is meant to be compiled at runtime for a vertex, fragment, geometry, tesselation, or compute shader.
* e.g.
*       LoadShaderAsString("./shaders/filepath");
*
* @param filename Path to the shader file
* @return Entire file stored as a single string
*/
std::string LoadShaderAsString(const std::string& filename){
    // Resulting shader program loaded as a single string
    std::string result = "";

    std::string line = "";
    std::ifstream myFile(filename.c_str());

    if(myFile.is_open()){
        while(std::getline(myFile, line)){
            result += line + '\n';
        }
        myFile.close();

    }

    return result;
}


/**
* CompileShader will compile any valid vertex, fragment, geometry, tesselation, or compute shader.
* e.g.
*       Compile a vertex shader:    CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
*       Compile a fragment shader:  CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
*
* @param type We use the 'type' field to determine which shader we are going to compile.
* @param source : The shader source code.
* @return id of the shaderObject
*/
GLuint CompileShader(GLuint type, const std::string& source){
    // Compile our shaders
    GLuint shaderObject;

    // Based on the type passed in, we create a shader object specifically for that
    // type.
    if(type == GL_VERTEX_SHADER){
        shaderObject = glCreateShader(GL_VERTEX_SHADER);
    }else if(type == GL_FRAGMENT_SHADER){
        shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    }

    const char* src = source.c_str();
    // The source of our shader
    glShaderSource(shaderObject, 1, &src, nullptr);
    // Now compile our shader
    glCompileShader(shaderObject);

    // Retrieve the result of our compilation
    int result;
    // Our goal with glGetShaderiv is to retrieve the compilation status
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

    if(result == GL_FALSE){
        int length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        char* errorMessages = new char[length]; // Could also use alloca here.
        glGetShaderInfoLog(shaderObject, length, &length, errorMessages);

        if(type == GL_VERTEX_SHADER){
            std::cout << "ERROR: GL_VERTEX_SHADER compilation failed!\n" << errorMessages << "\n";
        }else if(type == GL_FRAGMENT_SHADER){
            std::cout << "ERROR: GL_FRAGMENT_SHADER compilation failed!\n" << errorMessages << "\n";
        }
        // Reclaim our memory
        delete[] errorMessages;

        // Delete our broken shader
        glDeleteShader(shaderObject);

        return 0;
    }

  return shaderObject;
}



/**
* Creates a graphics program object (i.e. graphics pipeline) with a Vertex Shader and a Fragment Shader
*
* @param vertexShaderSource Vertex source code as a string
* @param fragmentShaderSource Fragment shader source code as a string
* @return id of the program Object
*/
GLuint CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource){

    // Create a new program object
    GLuint programObject = glCreateProgram();

    // Compile our shaders
    GLuint myVertexShader   = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Link our two shader programs together.
    // Consider this the equivalent of taking two .cpp files, and linking them into
    // one executable file.
    glAttachShader(programObject,myVertexShader);
    glAttachShader(programObject,myFragmentShader);
    glLinkProgram(programObject);

    // Validate our program
    glValidateProgram(programObject);

    // Once our final program Object has been created, we can
    // detach and then delete our individual shaders.
    glDetachShader(programObject,myVertexShader);
    glDetachShader(programObject,myFragmentShader);
    // Delete the individual shaders once we are done
    glDeleteShader(myVertexShader);
    glDeleteShader(myFragmentShader);

    return programObject;
}


/**
* Create the graphics pipeline
*
* @return void
*/
void CreateGraphicsPipeline(){

    std::string vertexShaderSource      = LoadShaderAsString("./shaders/vert.glsl");
    std::string fragmentShaderSource    = LoadShaderAsString("./shaders/frag.glsl");

    gGraphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource,fragmentShaderSource);
}


/**
* Initialization of the graphics application. Typically this will involve setting up a window
* and the OpenGL Context (with the appropriate version)
*
* @return void
*/
void InitializeProgram(){
    // Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO)< 0){
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << "\n";
        exit(1);
    }

    // Setup the OpenGL Context
    // Use OpenGL 4.1 core or greater
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    // We want to request a double buffer for smooth updating.
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create an application window using OpenGL that supports SDL
    gGraphicsApplicationWindow = SDL_CreateWindow( "Phong Illumination",
                                                    SDL_WINDOWPOS_UNDEFINED,
                                                    SDL_WINDOWPOS_UNDEFINED,
                                                    gScreenWidth,
                                                    gScreenHeight,
                                                    SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );

    // Check if Window did not create.
    if( gGraphicsApplicationWindow == nullptr ){
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
        exit(1);
    }

    // Create an OpenGL Graphics Context
    gOpenGLContext = SDL_GL_CreateContext( gGraphicsApplicationWindow );
    if( gOpenGLContext == nullptr){
        std::cout << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << "\n";
        exit(1);
    }

    // Initialize GLAD Library
    if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){
        std::cout << "glad did not initialize" << std::endl;
        exit(1);
    }

}

// Return a value that is a mapping between the current range and a new range.
// Source: https://www.arduino.cc/reference/en/language/functions/math/map/
float map_linear(float x, float in_min, float in_max, float out_min, float out_max){
    return (x-in_min) * (out_max - out_min) / (in_max - in_min) + out_min;;
}

// Pass in an unsigned integer representing the number of
// rows and columns in the plane (e.g. resolution=00)
// The plane is 'flat' so the 'y' position will be 0.0f;
std::vector<Triangle> generatePlane(size_t resolution = 1) {
    // Store the resulting plane
    std::vector<Triangle> result;

    float start = -1.0f;
    float end = 1.0f;
    float step = (end - start) / resolution;

    std::vector<Vertex> vertices;

    for (size_t i = 0; i <= resolution; ++i) {
        for (size_t j = 0; j <= resolution; ++j) {
            float x = start + j * step;
            float y = 0.0f;
            float z = start + i * step;

            float r = 0.68f;
            float g = 0.85f;
            float b = 1.0f;

            float nx = 0.0f;
            float ny = 1.0f;
            float nz = 0.0f;

            // Create vertex using constructor
            vertices.push_back(Vertex(
                x, y, z,          // position
                r, g, b,          // color
                nx, ny, nz,       // normal
                j/(float)resolution, i/(float)resolution  // texture coordinates
            ));
        }
    }

    for (size_t i = 0; i < resolution; ++i) {
        for (size_t j = 0; j < resolution; ++j) {
            Triangle triangle1;
            Triangle triangle2;
            size_t row1 = i * (resolution + 1);
            size_t row2 = (i + 1) * (resolution + 1);

            triangle1.vertices[0] = vertices[row1 + j];
            triangle1.vertices[1] = vertices[row2 + j];
            triangle1.vertices[2] = vertices[row1 + j + 1];

            triangle2.vertices[0] = vertices[row1 + j + 1];
            triangle2.vertices[1] = vertices[row2 + j];
            triangle2.vertices[2] = vertices[row2 + j + 1];

            result.push_back(triangle1);
            result.push_back(triangle2);
        }
    }

    return result;
}

void CreateLightBox() {
    // Cube vertices for the light box
    std::vector<GLfloat> lightBoxData = {
        // Front face
        -0.1f, -0.1f,  0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
         0.1f, -0.1f,  0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
         0.1f,  0.1f,  0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
         0.1f,  0.1f,  0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
        -0.1f,  0.1f,  0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
        -0.1f, -0.1f,  0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,  1.0f,

        // Back face
        -0.1f, -0.1f, -0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
         0.1f, -0.1f, -0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
         0.1f,  0.1f, -0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
         0.1f,  0.1f, -0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
        -0.1f,  0.1f, -0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
        -0.1f, -0.1f, -0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f, -1.0f,

        // Top face
        -0.1f,  0.1f, -0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
         0.1f,  0.1f, -0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
         0.1f,  0.1f,  0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
         0.1f,  0.1f,  0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
        -0.1f,  0.1f,  0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
        -0.1f,  0.1f, -0.1f,  1.0f, 1.0f, 1.0f,  0.0f,  1.0f,  0.0f,

        // Bottom face
        -0.1f, -0.1f, -0.1f,  1.0f, 1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
         0.1f, -0.1f, -0.1f,  1.0f, 1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
         0.1f, -0.1f,  0.1f,  1.0f, 1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
         0.1f, -0.1f,  0.1f,  1.0f, 1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
        -0.1f, -0.1f,  0.1f,  1.0f, 1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
        -0.1f, -0.1f, -0.1f,  1.0f, 1.0f, 1.0f,  0.0f, -1.0f,  0.0f,

        // Right face
         0.1f, -0.1f, -0.1f,  1.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.1f,  0.1f, -0.1f,  1.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.1f,  0.1f,  0.1f,  1.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.1f,  0.1f,  0.1f,  1.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.1f, -0.1f,  0.1f,  1.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.1f, -0.1f, -0.1f,  1.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,

        // Left face
        -0.1f, -0.1f, -0.1f,  1.0f, 1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.1f,  0.1f, -0.1f,  1.0f, 1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.1f,  0.1f,  0.1f,  1.0f, 1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.1f,  0.1f,  0.1f,  1.0f, 1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.1f, -0.1f,  0.1f,  1.0f, 1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.1f, -0.1f, -0.1f,  1.0f, 1.0f, 1.0f, -1.0f,  0.0f,  0.0f
    };

    gLightBoxVertices = lightBoxData.size() / 9;  // 9 values per vertex (pos, color, normal)

    // Generate and bind VAO
    glGenVertexArrays(1, &gVertexArrayObjectLight);
    glBindVertexArray(gVertexArrayObjectLight);

    // Generate and bind VBO
    glGenBuffers(1, &gVertexBufferObjectLight);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObjectLight);
    glBufferData(GL_ARRAY_BUFFER, lightBoxData.size() * sizeof(GLfloat), lightBoxData.data(), GL_STATIC_DRAW);

    // Set up vertex attributes
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 9, (void*)0);
    // Color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 9, (void*)(sizeof(GLfloat) * 3));
    // Normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 9, (void*)(sizeof(GLfloat) * 6));

    glBindVertexArray(0);
}

// Regenerate the flat plane
void GeneratePlaneBufferData() {
    // Generate a plane with the current resolution
    std::vector<Triangle> mesh = generatePlane(gFloorResolution);

    std::vector<GLfloat> vertexDataFloor;

    for (const auto& triangle : mesh) {
        for (const auto& vertex : triangle.vertices) {

            vertexDataFloor.push_back(vertex.x);
            vertexDataFloor.push_back(vertex.y);
            vertexDataFloor.push_back(vertex.z);
            vertexDataFloor.push_back(vertex.r);
            vertexDataFloor.push_back(vertex.g);
            vertexDataFloor.push_back(vertex.b);
            vertexDataFloor.push_back(vertex.nx);
            vertexDataFloor.push_back(vertex.ny);
            vertexDataFloor.push_back(vertex.nz);
        }
    }
    // Store size in a global so you can later determine how many
    // vertices to draw in glDrawArrays;
    // TODO: You need to verify to yourself if this 'size' represents the number of 'vertices' or not -- think about it.
    gFloorTriangles = vertexDataFloor.size() / 9;

    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObjectFloor);
    glBufferData(GL_ARRAY_BUFFER,                       // Kind of buffer we are working with
                            // (e.g. GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER)
                     vertexDataFloor.size() * sizeof(GL_FLOAT),     // Size of data in bytes
                     vertexDataFloor.data(),                                            // Raw array of data
                     GL_STATIC_DRAW);
}


/**
* Setup your geometry during the vertex specification step
*
* @return void
*/
void VertexSpecification() {
    glGenVertexArrays(1, &gVertexArrayObjectFloor);
    glBindVertexArray(gVertexArrayObjectFloor);
    glGenBuffers(1, &gVertexBufferObjectFloor);

    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObjectFloor);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 11, (void*)0);
    // Color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 11, (void*)(sizeof(GL_FLOAT) * 3));
    // Normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 11, (void*)(sizeof(GL_FLOAT) * 6));
    // Texture coordinates
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 11, (void*)(sizeof(GL_FLOAT) * 9));

    glBindVertexArray(0);

    // Model setup using OBJMesh class
    gModelTriangles = gMesh.GetTriangleCount() * 3;
    gMesh.SetupBuffers(gVertexArrayObjectModel, gVertexBufferObjectModel);

    CreateLightBox();
}


/**
* PreDraw
* Typically we will use this for setting some sort of 'state'
* Note: some of the calls may take place at different stages (post-processing) of the
*        pipeline.
* @return void
*/
void PreDraw(){
    // Disable depth test and face culling.
    glEnable(GL_DEPTH_TEST);                    // NOTE: Need to enable DEPTH Test
    glDisable(GL_CULL_FACE);

    // Set the polygon fill mode
    glPolygonMode(GL_FRONT_AND_BACK,gPolygonMode);

    // Initialize clear color
    // This is the background of the screen.
    glViewport(0, 0, gScreenWidth, gScreenHeight);
    glClearColor( 0.1f, 0.4f, 0.7f, 1.f );

    //Clear color buffer and Depth Buffer
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Use our shader
    glUseProgram(gGraphicsPipelineShaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gMesh.GetTextureID());

    // Model transformation by translating our object into world space
    glm::mat4 model = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,0.0f,0.0f));

    // Retrieve our location of our Model Matrix
    GLint u_ModelMatrixLocation = glGetUniformLocation( gGraphicsPipelineShaderProgram,"u_ModelMatrix");
    if(u_ModelMatrixLocation >=0){
        glUniformMatrix4fv(u_ModelMatrixLocation,1,GL_FALSE,&model[0][0]);
    }else{
        std::cout << "Could not find u_ModelMatrix, maybe a mispelling?\n";
        exit(EXIT_FAILURE);
    }


    // Update the View Matrix
    GLint u_ViewMatrixLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram,"u_ViewMatrix");
    if(u_ViewMatrixLocation>=0){
        glm::mat4 viewMatrix = gCamera.GetViewMatrix();
        glUniformMatrix4fv(u_ViewMatrixLocation,1,GL_FALSE,&viewMatrix[0][0]);
    }else{
        std::cout << "Could not find u_ViewMatrix, maybe a mispelling?\n";
        exit(EXIT_FAILURE);
    }


    // Projection matrix (in perspective)
    glm::mat4 perspective = glm::perspective(glm::radians(45.0f),
                                             (float)gScreenWidth/(float)gScreenHeight,
                                             0.1f,
                                             20.0f);

    // Retrieve our location of our perspective matrix uniform
    GLint u_ProjectionLocation= glGetUniformLocation( gGraphicsPipelineShaderProgram,"u_Projection");
    if(u_ProjectionLocation>=0){
        glUniformMatrix4fv(u_ProjectionLocation,1,GL_FALSE,&perspective[0][0]);
    }else{
        std::cout << "Could not find u_Projection, maybe a mispelling?\n";
        exit(EXIT_FAILURE);
    }

        GLint textureLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_texture");
    if(textureLocation >= 0) {
        glUniform1i(textureLocation, 0); // Use texture unit 0
    } else {
        std::cout << "Could not find u_texture uniform location\n";
    }

    float timeValue = SDL_GetTicks() / 1000.0f;

    // Compute moving light position
    float radius = 2.0f;
    float lightX = sin(timeValue) * radius;
    float lightY = 0.0f;
    float lightZ = cos(timeValue) * radius;
    glm::vec3 lightPos(lightX, lightY, lightZ);

    // Create model matrix for light box
glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), glm::vec3(lightX, lightY, lightZ));
lightModel = glm::scale(lightModel, glm::vec3(0.2f)); // Scale the light box

    // Set light position uniform
    GLint u_lightPosLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_lightPos");
    if (u_lightPosLocation >= 0) {
        glUniform3fv(u_lightPosLocation, 1, &lightPos[0]);
    }

    // Set light color uniform
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f); // White light
    GLint u_lightColorLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_lightColor");
    if (u_lightColorLocation >= 0) {
        glUniform3fv(u_lightColorLocation, 1, &lightColor[0]);
    }

    glm::vec3 materialAmbient(0.1f, 0.1f, 0.1f);
    glm::vec3 materialDiffuse(0.5f, 0.5f, 0.5f);
    glm::vec3 materialSpecular(1.0f, 1.0f, 1.0f);
    float materialShininess = 32.0f;

    GLint u_materialAmbientLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_materialAmbient");
    if (u_materialAmbientLocation >= 0) {
        glUniform3fv(u_materialAmbientLocation, 1, &materialAmbient[0]);
    }

    GLint u_materialDiffuseLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_materialDiffuse");
    if (u_materialDiffuseLocation >= 0) {
        glUniform3fv(u_materialDiffuseLocation, 1, &materialDiffuse[0]);
    }

    GLint u_materialSpecularLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_materialSpecular");
    if (u_materialSpecularLocation >= 0) {
        glUniform3fv(u_materialSpecularLocation, 1, &materialSpecular[0]);
    }

    GLint u_materialShininessLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_materialShininess");
    if (u_materialShininessLocation >= 0) {
        glUniform1f(u_materialShininessLocation, materialShininess);
    }

    glm::vec3 cameraPos = gCamera.GetEyePosition();
    GLint u_viewPosLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_viewPos");
    if (u_viewPosLocation >= 0) {
        glUniform3fv(u_viewPosLocation, 1, &cameraPos[0]);
    }

    GLint u_shadingModeLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_shadingMode");
    if (u_shadingModeLocation >= 0) {
        glUniform1i(u_shadingModeLocation, g_shadingMode);
    }

GLint u_LightModelMatrixLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_ModelMatrix");
}


/**
* Draw
* The render function gets called once per loop.
* Typically this includes 'glDraw' related calls, and the relevant setup of buffers
* for those calls.
*
* @return void
*/
void Draw() {
    // Draw floor
    glBindVertexArray(gVertexArrayObjectFloor);
    glDrawArrays(GL_TRIANGLES, 0, gFloorTriangles);

    // Draw model
    if (gRenderModel) {
        glBindVertexArray(gVertexArrayObjectModel);
        glDrawArrays(GL_TRIANGLES, 0, gModelTriangles);
    }

    // Draw light box with its own model matrix
    float timeValue = SDL_GetTicks() / 1000.0f;
    float radius = 2.0f;
    float lightX = sin(timeValue) * radius;
    float lightY = 0.0f;
    float lightZ = cos(timeValue) * radius;

    glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), glm::vec3(lightX, lightY, lightZ));
    lightModel = glm::scale(lightModel, glm::vec3(0.2f));

    GLint u_ModelMatrixLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_ModelMatrix");
    if(u_ModelMatrixLocation >= 0) {
        glUniformMatrix4fv(u_ModelMatrixLocation, 1, GL_FALSE, &lightModel[0][0]);
    }

    glBindVertexArray(gVertexArrayObjectLight);
    glDrawArrays(GL_TRIANGLES, 0, gLightBoxVertices);

    // Reset model matrix for next frame if needed
    glm::mat4 defaultModel = glm::mat4(1.0f);
    glUniformMatrix4fv(u_ModelMatrixLocation, 1, GL_FALSE, &defaultModel[0][0]);
}

/**
* Helper Function to get OpenGL Version Information
*
* @return void
*/
void getOpenGLVersionInfo(){
  std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
  std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
  std::cout << "Version: " << glGetString(GL_VERSION) << "\n";
  std::cout << "Shading language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
}


/**
* Function called in the Main application loop to handle user input
*
* @return void
*/
void Input(){
    // Two static variables to hold the mouse position
    static int mouseX=gScreenWidth/2;
    static int mouseY=gScreenHeight/2;

    // Event handler that handles various events in SDL
    // that are related to input and output
    SDL_Event e;
    //Handle events on queue
    while(SDL_PollEvent( &e ) != 0){
        // If users posts an event to quit
        // An example is hitting the "x" in the corner of the window.
        if(e.type == SDL_QUIT){
            std::cout << "Goodbye! (Leaving MainApplicationLoop())" << std::endl;
            gQuit = true;
        }
        if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE){
            std::cout << "ESC: Goodbye! (Leaving MainApplicationLoop())" << std::endl;
            gQuit = true;
        }
        if(e.type==SDL_MOUSEMOTION){
            // Capture the change in the mouse position
            mouseX+=e.motion.xrel;
            mouseY+=e.motion.yrel;
            gCamera.MouseLook(mouseX,mouseY);
        }
    }

    // Retrieve keyboard state
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_UP]) {
        SDL_Delay(250);
        gFloorResolution+=1;
        std::cout << "Resolution:" << gFloorResolution << std::endl;
        //GeneratePlaneBufferData();
    }
    if (state[SDL_SCANCODE_DOWN]) {
        SDL_Delay(250);
        gFloorResolution-=1;
        if(gFloorResolution<=1){
            gFloorResolution=1;
        }
        std::cout << "Resolution:" << gFloorResolution << std::endl;
        //GeneratePlaneBufferData();
    }

    // Camera
    // Update our position of the camera
    if (state[SDL_SCANCODE_W]) {
        gCamera.MoveForward(0.05f);
    }
    if (state[SDL_SCANCODE_S]) {
        gCamera.MoveBackward(0.05f);
    }
    if (state[SDL_SCANCODE_A]) {
        gCamera.MoveLeft(0.05f);
    }
    if (state[SDL_SCANCODE_D]) {
        gCamera.MoveRight(0.05f);
    }
    if (state[SDL_SCANCODE_1]) {
        SDL_Delay(250);
        gRenderModel = !gRenderModel;  // Toggle model rendering
        std::cout << "Model rendering: " << (gRenderModel ? "ON" : "OFF") << std::endl;
    }

    if (state[SDL_SCANCODE_TAB]) {
        SDL_Delay(250); // This is hacky in the name of simplicity,
                       // but we just delay the
                       // system by a few milli-seconds to process the
                       // keyboard input once at a time.
        if(gPolygonMode== GL_FILL){
            gPolygonMode = GL_LINE;
        }else{
            gPolygonMode = GL_FILL;
        }
    }

    if (state[SDL_SCANCODE_N]) {
        SDL_Delay(250);
        g_shadingMode = (g_shadingMode + 1) % 2;
        std::cout << "Shading mode: " << (g_shadingMode == 0 ? "Normals" : "Phong") << std::endl;
    }
}


/**
* Main Application Loop
* This is an infinite loop in our graphics application
*
* @return void
*/
void MainLoop(){

    // Little trick to map mouse to center of screen always.
    // Useful for handling 'mouselook'
    // This works because we effectively 're-center' our mouse at the start
    // of every frame prior to detecting any mouse motion.
    SDL_WarpMouseInWindow(gGraphicsApplicationWindow,gScreenWidth/2,gScreenHeight/2);
    SDL_SetRelativeMouseMode(SDL_TRUE);


    // While application is running
    while(!gQuit){
        // Handle Input
        Input();
        // Setup anything (i.e. OpenGL State) that needs to take
        // place before draw calls
        PreDraw();
        // Draw Calls in OpenGL
        // When we 'draw' in OpenGL, this activates the graphics pipeline.
        // i.e. when we use glDrawElements or glDrawArrays,
        //      The pipeline that is utilized is whatever 'glUseProgram' is
        //      currently binded.
        Draw();

        //Update screen of our specified window
        SDL_GL_SwapWindow(gGraphicsApplicationWindow);
    }
}



/**
* The last function called in the program
* This functions responsibility is to destroy any global
* objects in which we have create dmemory.
*
* @return void
*/
void CleanUp(){
    //Destroy our SDL2 Window
    SDL_DestroyWindow(gGraphicsApplicationWindow );
    gGraphicsApplicationWindow = nullptr;

    glDeleteBuffers(1, &gVertexBufferObjectFloor);
    glDeleteVertexArrays(1, &gVertexArrayObjectFloor);
    glDeleteBuffers(1, &gVertexBufferObjectModel);
    glDeleteVertexArrays(1, &gVertexArrayObjectModel);
    glDeleteBuffers(1, &gVertexBufferObjectLight);
    glDeleteVertexArrays(1, &gVertexArrayObjectLight);

    glDeleteProgram(gGraphicsPipelineShaderProgram);

    //Quit SDL subsystems
    SDL_Quit();
}


/**
* The entry point into our C++ programs.
*
* @return program status
*/
int main(int argc, char* args[]) {
    // Check command line arguments
    if (argc < 2) {
        std::cout << "Usage: " << args[0] << " <path_to_obj_file>\n";
        return 1;
    }

    // Print usage instructions
    std::cout << "Use w and s keys to move forward and back\n";
    std::cout << "Use a and d keys to move left and right\n";
    std::cout << "Use up and down to change tessellation\n";
    std::cout << "Use tab to toggle wireframe\n";
    std::cout << "Press 'n' to toggle shading mode (Normals/Phong)\n";
    std::cout << "Press ESC to quit\n";

    // 1. Initialize SDL and OpenGL context
    InitializeProgram();

    // 2. Create and compile shaders
    CreateGraphicsPipeline();

    // 3. Load the 3D model without textures
    std::string objFile = args[1];
    if (!gMesh.LoadOBJ(objFile)) {
        std::cout << "Failed to load OBJ file: " << objFile << "\n";
        return 1;
    }

    // 4. Now that OpenGL is initialized, load textures
    if (!gMesh.LoadTextures()) {
        std::cout << "Warning: Failed to load textures, continuing without textures\n";
    }

    // 5. Set up vertex buffers and attributes
    VertexSpecification();

    // 6. Generate any additional geometry (like the floor)
    GeneratePlaneBufferData();

    // 7. Enter the main application loop
    MainLoop();

    // 8. Clean up resources when the program ends
    CleanUp();

    return 0;
}
