#version 450 core 

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 TangentviewPos;
    vec3 TangentlightPos;
    vec3 TangentFragPos;
    mat3 TBN;  
} fs_in;

struct PointLight {
    vec3 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float constant;
    float linear;
    float quadratic;
};

in PointLight pt_light;

out vec4 color;    

struct Material {
    vec4 ambient;
    sampler2D diffuseTexture;
    sampler2D normalTexture;
    sampler2D depthTexture;
    vec4 specular;
    float shineness;
};

//uniform vec3 viewPos;

//uniform sampler2D diffuse_map;

uniform Material material;

uniform float height_scale;

vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 tex_coord)
{
   vec3 lightDir = normalize(light.position - fragPos );
   float diff = max(dot( normal, lightDir), 0.0);

   vec4 diff_color = texture( material.diffuseTexture, tex_coord );
   
   vec4 ambient = light.ambient * material.ambient;
   vec4 diffuse = light.diffuse * diff * diff_color;
    
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  

   float spec = pow( max( dot(normal, halfwayDir), 0.0), material.shineness ); 
   
   float distance = length(light.position - fragPos);
   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
   vec4 specular = light.specular * spec * material.specular; 
   ambient *= attenuation;
   diffuse *= attenuation;
   specular*= attenuation;
   
   return ambient + (diffuse + specular);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float height =  texture(material.depthTexture, texCoords).r;    
    vec2 p = viewDir.xy / viewDir.z * (height * height_scale);
    return texCoords - p;
}

void main(void) 
{  
    vec3 viewDir  = normalize( fs_in.TangentviewPos - fs_in.TangentFragPos);
    vec2 texCoords = ParallaxMapping( fs_in.TexCoords, viewDir );
    //vec2 texCoords = fs_in.TexCoords;
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;
    

    vec3 normal = texture(material.normalTexture, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(fs_in.TBN * normal);
    //vec3 normal  = normalize(fs_in.Normal);

    color = CalcPointLight(pt_light, normal, fs_in.TangentFragPos, viewDir, texCoords);
}