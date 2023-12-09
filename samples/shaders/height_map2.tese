#version 450 core

layout (triangles, equal_spacing, cw) in;

in vec4 pos_tc_out[];
in vec2 tex_tc_out[];

out vec2 tex_frag_in;
//out vec4 pos_frag_in[];

uniform mat4 mvp;
uniform sampler2D height_map;

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

vec2 interpolate3(in vec2 v0, in vec2 v1, in vec2 v2)
{
  return (gl_TessCoord.x * v0 + 
          gl_TessCoord.y * v1 + 
          gl_TessCoord.z * v2);
}

void main(void)
{ 
  //gl_Position = interpolate3(pos_tc_out[0], pos_tc_out[1], pos_tc_out[2]);
    vec4 p = interpolate3(pos_tc_out[0], pos_tc_out[1], pos_tc_out[2]);  
    vec2 t = interpolate3(tex_tc_out[0], tex_tc_out[1], tex_tc_out[2]);

    vec4 v = texture(height_map, t);              
    p.z = (v.r + v.g + v.b) * 0.3/3.0;

    gl_Position = mvp * p;
    
    tex_frag_in = t;

  //gl_Position = mvp * interpolate3(pos_tc_out[0], pos_tc_out[1], pos_tc_out[2]);
}