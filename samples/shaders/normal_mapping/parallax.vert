#version 450 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 vtex_coords;
layout (location = 3) in vec3 Tangent;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 TangentviewPos;
    vec3 TangentlightPos;
    vec3 TangentFragPos;
    mat3 TBN;
} vs_out;

struct PointLight {
    vec3 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float constant;
    float linear;
    float quadratic;
};

out PointLight pt_light;

uniform mat4 mvp;
uniform mat4 model;

uniform vec3 viewPos;

uniform PointLight light;
               
void main(void)
{  
    vs_out.FragPos = vec3(model * vec4(position, 1.0));
    
    mat3 inv_model = transpose(inverse(mat3(model)));
    vs_out.Normal  = inv_model * Normal;
    vs_out.TexCoords = vtex_coords;

    vec3 T = normalize(inv_model * Tangent);
    vec3 B = cross(vs_out.Normal, T); 
            //normalize(inv_model * Bitangent);

    vs_out.TBN = transpose( mat3( T, B, vs_out.Normal ) );  // Here TBN converts world space to tanget space

    pt_light = light;
    vs_out.TangentlightPos = vs_out.TBN * light.position;
    vs_out.TangentviewPos  = vs_out.TBN * viewPos;
    vs_out.TangentFragPos  = vs_out.TBN * vs_out.FragPos;

    gl_Position = mvp * vec4(position, 1.0);  
}