#version 450 core

layout (vertices = 3) out;

in vec4 pos_v[];
out vec4 pos_tc_out[];

void main(void){
    if (gl_InvocationID == 0){
        gl_TessLevelInner[0] = 4.0;
        gl_TessLevelOuter[0] = 4.0;
        gl_TessLevelOuter[1] = 1.0;
        gl_TessLevelOuter[2] = 4.0;    

        gl_TessLevelInner[1] = 1.0;
        gl_TessLevelOuter[3] = 1.0;
    } 

    pos_tc_out[gl_InvocationID] = pos_v[gl_InvocationID];
    
    //gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}