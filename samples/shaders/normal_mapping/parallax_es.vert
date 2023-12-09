layout (location = 0) in vec3 position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 vtex_coords;
layout (location = 3) in vec3 Tangent;

out vec3 fs_in_FragPos;
out vec3 fs_in_Normal;
out vec2 fs_in_TexCoords;
out vec3 fs_in_TangentviewPos;
out vec3 fs_in_TangentlightPos;
out vec3 fs_in_TangentFragPos;
out mat3 fs_in_TBN;

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
    fs_in_FragPos = vec3(model * vec4(position, 1.0));
    
    mat3 inv_model = transpose(inverse(mat3(model)));
    fs_in_Normal  = inv_model * Normal;
    fs_in_TexCoords = vtex_coords;

    vec3 T = normalize(inv_model * Tangent);
    vec3 B = cross(fs_in_Normal, T); 
            //normalize(inv_model * Bitangent);

    fs_in_TBN = transpose( mat3( T, B, fs_in_Normal ) );  // Here TBN converts world space to tanget space

    pt_light = light;
    fs_in_TangentlightPos = fs_in_TBN * light.position;
    fs_in_TangentviewPos  = fs_in_TBN * viewPos;
    fs_in_TangentFragPos  = fs_in_TBN * fs_in_FragPos;

    gl_Position = mvp * vec4(position, 1.0);  
}