// https://learnopengl.com/code_viewer_gh.php?code=src/5.advanced_lighting/3.1.2.shadow_mapping_base/3.1.2.shadow_mapping.vs

#version 330 core
layout (location = 0) in vec4 a_position;
layout (location = 1) in vec4 a_normal;
layout (location = 2) in vec4 a_color;

out VS_OUT {
    vec3 frag_position;
    vec3 normal;
    vec3 color;
    vec4 frag_light_space;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 light_space_matrix;

void main()
{
    vs_out.frag_position = vec3(model * vec4(a_position.xyz, 1.0));
    vs_out.normal = transpose(inverse(mat3(model))) * a_normal.xyz;
    vs_out.color = a_color.xyz;
    vs_out.frag_light_space = light_space_matrix * vec4(vs_out.frag_position, 1.0);
    gl_Position = projection * view * model * vec4(a_position.xyz, 1.0);
}