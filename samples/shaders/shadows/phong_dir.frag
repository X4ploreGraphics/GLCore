
struct DirLight {
    vec3 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

struct PointLight {
    vec3 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float cutOff;
    float outerCutOff;
};

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shineness;
};

#define NR_DIR_LIGHTS 1
#define NR_POINT_LIGHTS 1
#define NR_SPOT_LIGHTS 1

uniform DirLight dirLights[NR_DIR_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform float dir_count;
uniform float point_count;
uniform float spot_count;
uniform Material material;
uniform vec3 viewPos;

//uniform sampler2DShadow shadow_map;
uniform sampler2D shadow_map;

in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

out vec4 ifColor;


vec4 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow)
{
   vec3 lightDir = normalize(-light.direction);
   float diff = max(dot( normal, lightDir), 0.0);
    
    vec4 ambient = light.ambient * material.ambient;
    vec4 diffuse = light.diffuse * diff * material.diffuse;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow( max( dot(viewDir, reflectDir), 0.0), material.shineness );
    vec4 specular = light.specular * spec * material.specular;
    return (ambient + (1.0 - shadow) * (diffuse + specular) );
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    float currentDepth = projCoords.z; 
    float bias = 0.005;

    float closestDepth = texture(shadow_map, projCoords.xy).r;
    float shadow = 0.0;
    ivec2 tx_size = textureSize(shadow_map, 0);
    vec2 texelSize = vec2(1.0f / float(tx_size.x), 1.0f / float(tx_size.y));
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadow_map, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    //float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    if(projCoords.z > 1.0)
        shadow = 0.0;  

    return shadow;


    // Use with ShadowSampler
    /*float shadow = texture(shadow_map, vec3(projCoords.xy, currentDepth), bias);  //For ShadowMap Samplers
    if(projCoords.z > 1.0)
        shadow = 0.0;  

    return 1.0f - shadow;*/
}

void main()
{  
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);
  vec4 result = vec4(0, 0, 0, 0);
  float shadow = ShadowCalculation(FragPosLightSpace);

  for(int i = 0; i < int(dir_count); i++){
     result += CalcDirLight(dirLights[i], norm, viewDir, shadow); }
  //for(int j = 0; j < point_count; j++){
  //   result += CalcPointLight(pointLights[j], norm, FragPos, viewDir); }
  //for(int k = 0; k < spot_count; k++){
  //   result += CalcSpotLight(spotLights[k], norm, FragPos, viewDir, shadow);}

    

  ifColor = result;
  //ifColor = material.diffuse;
}
