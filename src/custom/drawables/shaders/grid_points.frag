
#version 330 core
out vec4 color;

in float v_iso_level;
in float v_origin_offset;
in float v_up_vector_offset;

uniform float iso_level;

void main()
{
    float offset = v_origin_offset;
    if(v_iso_level < iso_level) {
        offset = 1.0f - v_origin_offset;
    }

    // Turn point red if below iso_level
    color = vec4(vec3(1.0f - v_up_vector_offset, offset, offset), 1.0f);
}
