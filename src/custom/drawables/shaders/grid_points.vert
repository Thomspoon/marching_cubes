
#version 330 core
layout (location = 0) in vec4 a_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out float v_iso_level;

void main()
{
    v_iso_level = a_pos.w;
    gl_PointSize = 2.0f;
    gl_Position = projection * view * model * vec4(a_pos.xyz, 1.0f);
}
