#version 330 core
layout (location = 0) in vec4 a_position;
layout (location = 1) in vec4 a_normal;
layout (location = 2) in vec4 a_color;

uniform mat4 light_space_matrix;
uniform mat4 model;
uniform vec3 offset;

void main()
{
    gl_Position = light_space_matrix * model * vec4(a_position.xyz - offset, 1.0);
} 