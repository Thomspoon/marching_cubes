// Shader adapted from the following tutorials:

#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_texCoord;

out vec2 TexCoords;
out vec4 ParticleColor;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec4 color;

void main()
{
    TexCoords = a_texCoord;
    ParticleColor = color;
    gl_Position = projection * view * model * vec4(a_pos, 1.0f);
}
