// https://learnopengl.com/code_viewer_gh.php?code=src/5.advanced_lighting/3.1.2.shadow_mapping_base/3.1.2.shadow_mapping.fs
#version 330 core
out vec4 frag_color;

in VS_OUT {
    vec3 frag_position;
    vec3 normal;
    vec3 color;
    vec4 frag_light_space;
} fs_in;

uniform sampler2D shadowMap;

uniform vec3 light_pos;
uniform vec3 view_pos;
uniform vec3 light_color;

float shadow_calculation(vec4 frag_light_space)
{
   // perform perspective divide
   vec3 projection_coords = frag_light_space.xyz / frag_light_space.w;

   // transform to [0,1] range
   projection_coords = projection_coords * 0.5 + 0.5;

   // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
   float closest_depth = texture(shadowMap, projection_coords.xy).r; 

   // get depth of current fragment from light's perspective
   float current_depth = projection_coords.z;

   // check whether current frag pos is in shadow
   float shadow = current_depth > closest_depth  ? 1.0 : 0.0;

   return shadow;
}

void main()
{           
   // ambient
   vec3 ambient = 0.1 * fs_in.color;
   vec3 normal = normalize(fs_in.normal);

   // diffuse
   vec3 lightDir = normalize(light_pos - fs_in.frag_position);
   float diff = max(dot(lightDir, normal), 0.0);
   vec3 diffuse = diff * light_color;

   // specular
   vec3 viewDir = normalize(view_pos - fs_in.frag_position);
   vec3 reflectDir = reflect(-lightDir, normal);
   float spec = 0.0;
   vec3 halfwayDir = normalize(lightDir + viewDir);  
   spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
   vec3 specular = spec * light_color;

   // calculate shadow
   float shadow = shadow_calculation(fs_in.frag_light_space);                      
   vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * fs_in.color;    

   frag_color = vec4(lighting, 1.0);
}

