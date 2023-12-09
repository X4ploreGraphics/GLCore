#version 450 core

layout(location = 0) in vec3 vPosition;
layout(location = 2) in vec3 vNormals;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 model;
uniform mat4 inv_model;

uniform mat4 lightSpaceMatrix;

out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;

void  main()
{
  gl_Position = mvp * vec4(vPosition, 1.0f);
  Normal =  mat3(inv_model) * vNormals ; 
  FragPos = vec3(model * vec4(vPosition, 1.0) );
  FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
}