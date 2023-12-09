#version 450 core

layout (vertices = 3) out;

in vec2 VTexCoords[];
in vec4 pos_v[];

out vec4 pos_tc_out[];
out vec2 tex_tc_out[];

void main(void){
    if (gl_InvocationID == 0){
        gl_TessLevelInner[0] = 10.0;
        gl_TessLevelOuter[0] = 10.0;
        gl_TessLevelOuter[1] = 10.0;
        gl_TessLevelOuter[2] = 10.0;    

        gl_TessLevelInner[1] = 10.0;
        gl_TessLevelOuter[3] = 10.0;
    } 

    pos_tc_out[gl_InvocationID] = pos_v[gl_InvocationID];
    tex_tc_out[gl_InvocationID] = VTexCoords[gl_InvocationID];
    
    //gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}