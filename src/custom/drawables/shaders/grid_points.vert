
#version 330 core
layout (location = 0) in vec4 a_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out float v_origin_offset;
out float v_up_vector_offset;
out float v_iso_level;

void main()
{
    vec3 delta1 = abs(a_pos.xyz - vec3(0.0, 0.0, 0.0)); // Get delta from middle vec3
    vec3 delta2 = abs(a_pos.xyz - vec3(0.0, 1.0, 0.0)); // Get delta from middle vec3

    // Color origin light red
    v_origin_offset = 1.0f;
    if ((delta1.x <= 0.05) && (delta1.y <= 0.05) && (delta1.z <= 0.05))
    {
        v_origin_offset = 0.5f;
    }

    // Color y = 1 x = 0 z = 0 blue to show where up vector is
    v_up_vector_offset = 0.0f;
    if ((delta2.x <= 0.05) && (delta2.y <= 0.05) && (delta2.z <= 0.05))
    {
        v_up_vector_offset = 1.0f;
    }

    v_iso_level = a_pos.w;
    gl_PointSize = 4.0f;
    gl_Position = projection * view * model * vec4(a_pos.xyz, 1.0f);
}
