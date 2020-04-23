// Shader adapted from the following tutorials:
// https://learnopengl.com/Lighting/Basic-Lighting

#version 330 core
layout (location = 0) in vec4 a_pos;
layout (location = 1) in vec4 a_normal;
layout (location = 2) in vec4 a_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 light_pos;
uniform vec3 view_pos;
uniform vec3 light_color;

out vec3 lighting;

void main()
{
    gl_Position = projection * view * model * vec4(a_pos.xyz, 1.0f);

    // Gourad shading
    vec3 position = vec3(model * vec4(a_pos.xyz, 1.0));
    vec3 normal = mat3(transpose(inverse(model))) * a_normal.xyz;

    // Ambient lighting
    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * light_color;

    // diffuse 
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - position);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;
    
    // specular
    float specular_strength = 0.5;
    vec3 view_dir = normalize(view_pos - position);
    vec3 reflect_dir = reflect(-light_dir, norm);  
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = specular_strength * spec * light_color;  
        
    lighting = (ambient + diffuse + specular) * a_color.xyz;
}
