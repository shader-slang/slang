//TEST:SIMPLE(filecheck=CHECKGLSLANG): -target spirv -stage compute -entry main -allow-glsl
//TEST:SIMPLE(filecheck=CHECKDIRECT): -target spirv -stage compute -entry main -allow-glsl -emit-spirv-directly
#version 430
precision highp float;
precision highp int;

layout(binding = 0) buffer MyBlockName
{
    vec4 data[];
} output_data;

// CHECKGLSLANG-DAG: [[x:%[^ ]+]] = OpConstant {{%[^ ]+}} 44
// CHECKGLSLANG-DAG: [[y:%[^ ]+]] = OpConstant {{%[^ ]+}} 45
// CHECKGLSLANG-DAG: [[z:%[^ ]+]] = OpConstant {{%[^ ]+}} 46
// CHECKGLSLANG: %gl_WorkGroupSize = OpConstantComposite {{%[^ ]+}} [[x]] [[y]] [[z]]

// CHECKDIRECT: OpExecutionMode %main LocalSize 44 45 46
layout(local_size_x = 44, local_size_y = 45, local_size_z = 46) in;
void main()
{
    output_data.data[gl_GlobalInvocationID.x] = vec4(gl_GlobalInvocationID, 1);
}
