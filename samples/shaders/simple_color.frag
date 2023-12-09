in vec2 VTexCoords;
out vec4 color;     
uniform vec4 d_color;

void main(void) 
{  
    color = d_color;
    //color = vec4(texture(diffuse_map, VTexCoords) );
    //color = vec4(VColor, 1.0);
}