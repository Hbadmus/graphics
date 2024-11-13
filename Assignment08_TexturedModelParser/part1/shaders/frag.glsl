#version 410 core

// Input from vertex shader
in vec2 v_texCoord;
in vec3 v_normal;
in vec3 v_fragPos;
in vec3 v_color;


// Uniforms
uniform sampler2D u_texture;        // Added texture sampler
uniform vec3 u_lightPos;
uniform vec3 u_viewPos;
uniform vec3 u_lightColor;
uniform vec3 u_materialAmbient;
uniform vec3 u_materialDiffuse;
uniform vec3 u_materialSpecular;
uniform float u_materialShininess;
uniform int u_shadingMode;
uniform int u_polygonMode;          // For wireframe toggle

// Output
out vec4 FragColor;

void main()
{
    if (u_polygonMode == 2) {  // GL_LINE mode
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);  // White wireframe
        return;
    }

    // Get texture color
    vec3 textureColor = texture(u_texture, v_texCoord).rgb;

    if (u_shadingMode == 0) {  // Normal visualization mode
        FragColor = vec4(normalize(v_normal) * 0.5 + 0.5, 1.0);
        return;
    }

    // Phong lighting calculation
    vec3 norm = normalize(v_normal);
    vec3 lightDir = normalize(u_lightPos - v_fragPos);

    // Ambient
    vec3 ambient = u_materialAmbient * u_lightColor;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = u_materialDiffuse * diff * u_lightColor;

    // Specular
    vec3 viewDir = normalize(u_viewPos - v_fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_materialShininess);
    vec3 specular = u_materialSpecular * spec * u_lightColor;

    // Combine lighting with texture color
    vec3 result = textureColor;
    FragColor = vec4(result, 1.0);
}
