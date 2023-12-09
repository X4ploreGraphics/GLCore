
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormals;


uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 model;
uniform mat4 inv_model;

uniform mat4 projection;
uniform mat4 view;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    //vec2 TexCoords;
} vs_out;

void  main()
{
  gl_Position = mvp * vec4(vPosition, 1.0f);
  vs_out.FragPos = vec3(model * vec4(vPosition, 1.0));
  //vs_out.Normal =  mat3(inv_model) * vNormals ; 
  vs_out.Normal =  transpose( inverse(mat3(model)) ) * vNormals ; 
  //vs_out.TexCoords = aTexCoords;
  //FragPos = vec3(model * vec4(vPosition, 1.0) );
  //FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
}