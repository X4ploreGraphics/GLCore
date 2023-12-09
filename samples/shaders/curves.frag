#version 450 core 
out vec4 color;     
in vec4 cc;

void main(void) 
{  
    color = vec4(1.0, 0.5, 0.0, 1.0);
    //color = cc;
    //color = vec4(texture(diffuse_map, VTexCoords));
    //color = vec4(VColor, 1.0);
}