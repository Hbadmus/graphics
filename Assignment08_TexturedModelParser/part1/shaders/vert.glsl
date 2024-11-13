#version 410 core

// Input vertex data
layout(location=0) in vec3 position;
layout(location=1) in vec3 color;
layout(location=2) in vec3 normal;
layout(location=3) in vec2 texCoord;

// Uniforms for transformations
uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_Projection;

// Output to fragment shader
out vec3 v_color;
out vec2 v_texCoord;
out vec3 v_normal;
out vec3 v_fragPos;

void main()
{
    // Transform vertex position to world space
    vec4 worldPos = u_ModelMatrix * vec4(position, 1.0);
    v_fragPos = vec3(worldPos);

    // Transform normal to world space
    // Note: Using the normal matrix (transpose(inverse(model))) to handle non-uniform scaling
    v_normal = normalize(mat3(transpose(inverse(u_ModelMatrix))) * normal);

    // Pass through color
    v_color = color;

    // Pass through texture coordinates - ensure they're properly oriented
    v_texCoord = texCoord;

    // Final position
    gl_Position = u_Projection * u_ViewMatrix * worldPos;
}
