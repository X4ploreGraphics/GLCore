#version 450 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D hdrBuffer;

void main()
{
    const float gamma = 1.0;
    const float exposure = 1.0f;             //Range 0 ~ 1, can be made to uniform
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;

    vec3 mapped = vec3(1.0) - exp( -hdrColor * exposure);
    //vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
    //FragColor = vec4(hdrColor, 1.0);
}