//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry main -allow-glsl
#version 430
precision highp float;
precision highp int;

layout(binding = 0) buffer MyBlockName
{
    vec4 a;
    float b;
    int c;
} output_data;

layout(local_size_x = 4) in;
void main()
{
    output_data.a = vec4(gl_GlobalInvocationID, 1);
    output_data.b = 10;
    output_data.c = 20;
    // CHECK: OpEntryPoint
}
