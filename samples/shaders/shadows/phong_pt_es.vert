
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormals;


uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 model;
uniform mat4 inv_model;

uniform mat4 projection;
uniform mat4 view;


out vec3 fs_in_FragPos;
out vec3 fs_in_Normal;

void  main()
{
  gl_Position = mvp * vec4(vPosition, 1.0f);
  fs_in_FragPos = vec3(model * vec4(vPosition, 1.0));
  //fs_in_Normal =  mat3(inv_model) * vNormals ; 
  fs_in_Normal =  transpose( inverse(mat3(model)) ) * vNormals ; 
  //fs_in_TexCoords = aTexCoords;
  //FragPos = vec3(model * vec4(vPosition, 1.0) );
  //FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
}