// An explicit `local_size_x` is the unspecialized size of an axis that a separate
// `local_size_x_id` statement makes specializable, as in GL_KHR_vulkan_glsl.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -source-language glsl

#version 450

layout(local_size_x = 4, local_size_y = 2) in;
layout(local_size_x_id = 18) in;
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
layout(std430, binding = 0) buffer OutputBlock { uint values[]; } outputBuffer;
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
// BUF: 4
// BUF-NEXT: 2
// BUF-NEXT: 1
// BUF-NEXT: 8
