layout (location = 0)  in vec3 position;
layout (location = 1)  in vec2 vtex_coords;
out vec2 VTexCoords;
out vec3 VColor;

uniform mat4 mvp;
uniform sampler2D height_map;
                
void main(void)
{  
    vec3 p = position;
    if(position.x > 0.0 && position.y > 0.0){
      p.z = 1.0;
      VColor = vec3(1.0, 0.5, 0.0);
    }
    else{
      p.z = -1.0;
      VColor = vec3(0.0, 0.5, 0.0);
    }
      
    vec4 v = texture(height_map, vtex_coords);              
    //p.z = v.r;
    p.z = (v.r + v.g + v.b) * 0.5/3.0;
   
   gl_Position = mvp * vec4(p, 1.0);  
   
   VTexCoords = vtex_coords;
}