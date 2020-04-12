// Shader adapted from the following tutorials:
// https://learnopengl.com/Lighting/Basic-Lighting

#version 330 core
layout (location = 0) in vec4 a_pos;
layout (location = 1) in vec4 a_normal;
layout (location = 2) in vec4 a_color;

out vec3 fragment_pos;
out vec3 surface_normal;
out vec3 surface_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    fragment_pos = vec3(model * vec4(a_pos.xyz, 1.0f));
    surface_normal = mat3(transpose(inverse(model))) * a_normal.xyz;
    surface_color = a_color.xyz;

    gl_Position = projection * view * model * vec4(a_pos.xyz, 1.0f);
}
