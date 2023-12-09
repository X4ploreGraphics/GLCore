#version 450 core

layout (triangles, equal_spacing, cw) in;

in vec4 pos_tc_out[];
//out vec4 pos_frag_in[];

//quad interpolate
vec4 interpolate4(in vec4 v0, in vec4 v1, in vec4 v2, in vec4 v3)
{
  vec4 a = mix(v0, v1, gl_TessCoord.x);
  vec4 b = mix(v3, v2, gl_TessCoord.x);
  return mix(a, b, gl_TessCoord.y);
}

//traingle interpolate
vec4 interpolate3(in vec4 v0, in vec4 v1, in vec4 v2)
{
  return (gl_TessCoord.x * v0 + 
          gl_TessCoord.y * v1 + 
          gl_TessCoord.z * v2);
}

void main(void)
{ 
  gl_Position = interpolate3(pos_tc_out[0], pos_tc_out[1], pos_tc_out[2]);
}