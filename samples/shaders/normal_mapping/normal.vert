layout (location = 0) in vec3 position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 vtex_coords;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 Bitangent;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    mat3 TBN;
} vs_out;

uniform mat4 mvp;
uniform mat4 model;
               
void main(void)
{  
    vs_out.FragPos = vec3(model * vec4(position, 1.0));
    
    mat3 inv_model = transpose(inverse(mat3(model)));
    vs_out.Normal  = inv_model * Normal;
    vs_out.TexCoords = vtex_coords;

    vec3 T = normalize(inv_model * Tangent);
    vec3 B = cross(vs_out.Normal, T); 
            //normalize(inv_model * Bitangent);

    vs_out.TBN = mat3( T, B, vs_out.Normal );

    gl_Position = mvp * vec4(position, 1.0);  
}