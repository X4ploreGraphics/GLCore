#version 450 core
layout (location = 0)  in vec3 position;

out vec4 pos_v;

uniform mat4 mvp;
                
void main(void)
{  
   vec4 p = vec4(position, 1.0);   
   //gl_Position = p;
   //pos_v = p;
   pos_v = mvp * p;  
}