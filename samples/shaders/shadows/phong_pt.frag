
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
    vec4 specular;
    float shineness;
};

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    //vec2 TexCoords;
} fs_in;

out vec4 ifColor;

#define NR_POINT_LIGHTS 1

uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform float point_count;

uniform Material material;

uniform vec3 viewPos;
uniform vec3 lightPos;

uniform samplerCube depthMap;
//uniform sampler2D diffuseTexture;

uniform float far_plane;

vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow)
{
   vec3 lightDir = normalize(light.position - fragPos );
   float diff = max(dot( normal, lightDir), 0.0);
   
   vec4 ambient = light.ambient * material.ambient;
   vec4 diffuse = light.diffuse * diff * material.diffuse;
    
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  

   float spec = pow( max( dot(normal, halfwayDir), 0.0), material.shineness ); 
   
   float distance = length(light.position - fragPos);
   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
   vec4 specular = light.specular * spec * material.specular; 
   ambient *= attenuation;
   diffuse *= attenuation;
   specular*= attenuation;
   
   return ambient + (diffuse + specular) * (1.0 - shadow);
}

float ShadowCalculation(vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;
    // ise the fragment to light vector to sample from the depth map    
    float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    //float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    //float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;        
    
    // display closestDepth as debug (to visualize depth cubemap)
     //ifColor = vec4(vec3(closestDepth / far_plane), 1.0);  


    vec3 sampleOffsetDirections[20] = vec3[]
    (
       vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
       vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
       vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
       vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
       vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    ); 

    float shadow = 0.0;
    float bias   = 0.15;
    int samples  = 20;
    float viewDistance = length(viewPos - fragPos);
    //float diskRadius = 0.05;
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;  
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= far_plane;   // Undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);  
        
    return shadow;
}

void main()
{  
  vec3 norm = normalize(fs_in.Normal);
  vec3 viewDir = normalize(viewPos - fs_in.FragPos);
  vec4 result = vec4(0, 0, 0, 0);
  float shadow = ShadowCalculation(fs_in.FragPos);

  for(int k = 0; k < point_count; k++){
     result += CalcPointLight(pointLights[k], norm, fs_in.FragPos, viewDir, shadow);}

  ifColor = result;
  //ifColor = material.diffuse;
}
