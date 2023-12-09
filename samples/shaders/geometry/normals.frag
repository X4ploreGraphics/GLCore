out vec4 color;

uniform vec4  cc;

void main(void) 
{  
    color = cc;
    //color = vec4(texture(diffuse_map, VTexCoords));
    //color = vec4(VColor, 1.0);
}