#version 450 core

layout(location = 0) in vec3 vPosition;
layout(location = 2) in vec2 vtex_coords;

out vec2 TexCoords;

void  main()
{
  gl_Position = vec4(vPosition, 1.0f);
  TexCoords = vtex_coords;
}