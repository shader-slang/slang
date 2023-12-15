//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry main -allow-glsl
#version 430
precision highp float;
precision highp int;

layout(binding = 0) buffer MyBlockName
{
    vec4 data[];
} output_data;

layout(local_size_x = 4) in;
void main()
{
    output_data.data[gl_GlobalInvocationID.x] = vec4(gl_GlobalInvocationID, 1);
    // CHECK: OpEntryPoint
}
