//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv -emit-spirv-directly

// missing implementation of most builtin values due to non trivial translation
//DISABLE_TEST:SIMPLE(filecheck=CHECK_HLSL): -allow-glsl -stage compute -entry computeMain -target hlsl -DTARGET_HLSL
// missing implementation of most builtin values due to non trivial translation
//DISABLE_TEST:SIMPLE(filecheck=CHECK_CUDA): -allow-glsl -stage compute -entry computeMain -target cuda -DTARGET_CUDA 
//missing implementation of system (varying?) values
//DISABLE_TEST:SIMPLE(filecheck=CHECK_CPP):  -allow-glsl -stage compute -entry computeMain -target cpp -DTARGET_CPP

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly
#version 430

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[];
} outputBuffer;

layout(local_size_x = 32) in;

void computeMain()
{
    if (gl_GlobalInvocationID.x == 3) {
        outputBuffer.data[0] = true
            && gl_NumSubgroups == 1
            && gl_SubgroupID  == 0 //1 subgroup, 0 based indexing
            && gl_SubgroupSize == 32
            && gl_SubgroupInvocationID == 3
            && gl_SubgroupEqMask == uvec4(0b1000,0,0,0)
            && gl_SubgroupGeMask == uvec4(0xFFFFFFF8,0,0,0)
            && gl_SubgroupGtMask == uvec4(0xFFFFFFF0,0,0,0)
            && gl_SubgroupLeMask == uvec4(0b1111,0,0,0)
            && gl_SubgroupLtMask == uvec4(0b111,0,0,0)
            ;
    }
    // CHECK_GLSL: void main(
    // CHECK_SPV: OpEntryPoint
    // CHECK_HLSL: void computeMain(
    // CHECK_CUDA: void computeMain(
    // CHECK_CPP: void _computeMain(
    // BUF: 1
}
