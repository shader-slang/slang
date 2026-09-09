//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl/compute-shader-layout.slang -target spirv -stage compute -entry main
//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl/compute-shader-layout.slang -target spirv -stage compute -entry main -emit-spirv-directly
#version 430
precision highp float;
precision highp int;

layout(binding = 0) buffer MyBlockName
{
    vec4 data[];
} output_data;

// CHECK: OpExecutionMode %main LocalSize 44 45 46
layout(local_size_x = 44, local_size_y = 45, local_size_z = 46) in;
void main()
{
    output_data.data[gl_GlobalInvocationID.x] = vec4(gl_GlobalInvocationID, 1);
}
