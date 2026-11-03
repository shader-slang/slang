// An unspecialized dispatch gives each `local_size_{xyz}_id` axis without a `constant_id`
// declaration the default workgroup size of 1.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -source-language glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -source-language glsl -emit-spirv-via-glsl

#version 450

layout(local_size_x_id = 5, local_size_y_id = 6, local_size_z = 2) in;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
layout(std430, binding = 0) buffer OutputBlock
{
    uint values[];
} outputBuffer;

[shader("compute")]
void computeMain()
{
    if (gl_LocalInvocationIndex == 0)
    {
        outputBuffer.values[0] = gl_WorkGroupSize.x;
        outputBuffer.values[1] = gl_WorkGroupSize.y;
        outputBuffer.values[2] = gl_WorkGroupSize.z;
    }
    atomicAdd(outputBuffer.values[3], 1);
}

// BUF: 1
// BUF-NEXT: 1
// BUF-NEXT: 2
// BUF-NEXT: 2
