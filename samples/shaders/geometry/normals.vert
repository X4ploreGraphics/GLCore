layout (location = 0)  in vec3 position;
layout (location = 1)  in vec3 normals;

out vec4 normal;

uniform mat4 mvp;
                
void main(void)
{  
    vec4 p = vec4(position, 1.0);
   gl_Position = mvp * p;  
   normal = mvp * vec4(normals, 1.0);
   //normal = vec4(normals, 1.0);
}