#version 450 core 
in vec2 tex_frag_in;
out vec4 color;     
uniform sampler2D height_map;
uniform sampler2D diffuse_map;

void main(void) 
{  
    //color = vec4(1.0, 0.5, 0.0, 1.0);
    color = vec4(texture(diffuse_map, tex_frag_in));
    //color = vec4(VColor, 1.0);
}