
in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

out vec4 color;    

struct PointLight {
    vec3 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float constant;
    float linear;
    float quadratic;
};

struct Material {
    vec4 ambient;
    sampler2D diffuseTexture;
    sampler2D normalTexture;
    vec4 specular;
    float shineness;
};

uniform vec3 viewPos;

//uniform sampler2D diffuse_map;
uniform sampler2D normalMap;

uniform PointLight light;
uniform Material material;

vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
   vec3 lightDir = normalize(light.position - fragPos );
   float diff = max(dot( normal, lightDir), 0.0);

   vec4 diff_color = texture( material.diffuseTexture, fs_in.TexCoords );
   
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

void main(void) 
{  
    vec3 normal = texture(material.normalTexture, fs_in.TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(fs_in.TBN * normal);
    //vec3 normal  = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    color = CalcPointLight(light, normal, fs_in.FragPos, viewDir);
    //vec4( texture(material.diffuseTexture, fs_in.TexCoords) );
}