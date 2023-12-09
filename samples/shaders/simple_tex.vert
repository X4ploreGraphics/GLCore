layout (location = 0)  in vec3 position;
layout (location = 1)  in vec2 vtex_coords;
out vec2 VTexCoords;
uniform mat4 mvp;
               
void main(void)
{  
    vec3 p = position;
    p = 0.8 * p;
   gl_Position = vec4(p, 1.0) + vec4(0.3, 0.3, 0.0, 0.0);  
   VTexCoords = vtex_coords;
}