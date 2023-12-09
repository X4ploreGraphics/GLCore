#version 450 core
out vec4 FragColor;

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
    vec4 diffuse;
    sampler2D diffuseTexture;
    vec4 specular;
    float shineness;
};

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform Material material;
uniform PointLight light[4];

uniform int light_count;

uniform bool texture_on;

uniform vec3 viewPos;

vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
   vec3 lightDir = normalize(light.position - fragPos );
   float diff = max(dot( normal, lightDir), 0.0);

   vec4 diff_color = texture_on ? texture( material.diffuseTexture, fs_in.TexCoords ) : material.diffuse;
   
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


void main()
{           
    vec3 normal = normalize(fs_in.Normal);    
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    vec4 result = vec4(0, 0, 0, 0);
    for(int i = 0; i < light_count; i++)
    {
        result += CalcPointLight(light[i], normal, fs_in.FragPos, viewDir); 
    }
    
    FragColor = result;
    //CalcPointLight(light, normal, fs_in.FragPos, viewDir); 
}