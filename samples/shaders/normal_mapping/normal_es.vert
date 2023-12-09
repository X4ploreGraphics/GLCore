layout (location = 0) in vec3 position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 vtex_coords;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 Bitangent;

out vec3 fs_in_FragPos;
out vec3 fs_in_Normal;
out vec2 fs_in_TexCoords;
out mat3 fs_in_TBN;

uniform mat4 mvp;
uniform mat4 model;
               
void main(void)
{  
    fs_in_FragPos = vec3(model * vec4(position, 1.0));
    
    mat3 inv_model = transpose(inverse(mat3(model)));
    fs_in_Normal  = inv_model * Normal;
    fs_in_TexCoords = vtex_coords;

    vec3 T = normalize(inv_model * Tangent);
    vec3 B = cross(fs_in_Normal, T); 
            //normalize(inv_model * Bitangent);

    fs_in_TBN = mat3( T, B, fs_in_Normal );

    gl_Position = mvp * vec4(position, 1.0);  
}