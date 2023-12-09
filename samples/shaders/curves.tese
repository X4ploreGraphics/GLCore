#version 450 core

layout (isolines, equal_spacing, cw) in;

in vec4 pos_tc_out[];
//out vec4 pos_frag_in[];

uniform mat4 mvp;

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

vec4 plane_curve(float u,  in vec4 p1, in vec4 p2, in vec4 p3)
{
	vec4 p = (2*u*u - 3*u + 1) * p1;
		p += (-4*u*u + 4*u) * p2;
		p += (2*u*u - u) * p3;

	return p;
}

void main(void)
{ 
  //gl_Position = interpolate3(pos_tc_out[0], pos_tc_out[1], pos_tc_out[2]);
  float u = gl_TessCoord.x;
  gl_Position = plane_curve(u, pos_tc_out[0], pos_tc_out[1], pos_tc_out[2]);
}