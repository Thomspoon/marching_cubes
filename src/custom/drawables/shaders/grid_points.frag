
#version 330 core
out vec4 color;

in float v_iso_level;

uniform float iso_level;

void main()
{
    // Turn point red if below iso_level
    if(v_iso_level > iso_level)
    {
        color = vec4(0.4f, 0.4f, 0.4f, 0.4f);
    }
    else
    {
        color = vec4(1.0f, 0.5f, 0.5f, 0.5f);
    }
}
