// Shader adapted from the following tutorials:
// https://learnopengl.com/Lighting/Basic-Lighting

#version 330 core
out vec4 color;

in vec3 lighting; 

void main()
{
   color = vec4(lighting, 1.0);
}