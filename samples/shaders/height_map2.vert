#version 450 core
layout (location = 0)  in vec3 position;
layout (location = 1)  in vec2 vtex_coords;

out vec2 VTexCoords;
out vec4 pos_v;

//uniform mat4 mvp;
//uniform sampler2D height_map;
                
void main(void)
{  
    pos_v = vec4(position, 1.0);
   //gl_Position = mvp * vec4(p, 1.0);  
   
   VTexCoords = vtex_coords;
}