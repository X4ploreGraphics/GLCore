in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

out vec4 cc;

void main()
{
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;

    cc = vec4(lightDistance, lightDistance, lightDistance, 1);
}