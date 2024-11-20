// ==================================================================
#version 330 core

// The final output color of each 'fragment' from our fragment shader.
out vec4 FragColor;

// Take in our previous texture coordinates.
in vec3 FragPos;
in vec2 v_texCoord;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

// If we have texture coordinates, they are stored in this sampler.
uniform sampler2D u_DiffuseMap; 
uniform sampler2D u_NormalMap; 

void main()
{
	// Store the texture coordinates
	vec3 normal = texture(u_NormalMap, v_texCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    vec3 color = texture(u_DiffuseMap, v_texCoord).rgb;

    vec3 ambient = 0.1 * color;

    vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * color;

    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    vec3 specular = spec * vec3(0.2);

    vec3 result = ambient + diffuse + specular;

    FragColor = vec4(result, 1.0);
}
