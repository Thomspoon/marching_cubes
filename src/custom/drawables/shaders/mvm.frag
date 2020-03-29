
#version 330 core
out vec4 color;

void main()
{
    // Turn point red if below iso_level
    color = vec4(vec3(1.0f), 1.0f);
}
