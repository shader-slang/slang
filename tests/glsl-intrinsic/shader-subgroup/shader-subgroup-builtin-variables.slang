//TEST:SIMPLE_EX(filecheck=CHECK_GLSL): -lang glsl tests/glsl-intrinsic/shader-subgroup/shader-subgroup-builtin-variables.slang -stage compute -entry computeMain -target glsl -DTARGET_VK
//TEST:SIMPLE_EX(filecheck=CHECK_SPV): -lang glsl tests/glsl-intrinsic/shader-subgroup/shader-subgroup-builtin-variables.slang -stage compute -entry computeMain -target spirv -emit-spirv-directly -DTARGET_VK

// missing implementation of most builtin values due to non trivial translation
//DISABLE_TEST:SIMPLE_EX(filecheck=CHECK_HLSL): -lang glsl tests/glsl-intrinsic/shader-subgroup/shader-subgroup-builtin-variables.slang -stage compute -entry computeMain -target hlsl -DTARGET_HLSL
// missing implementation of most builtin values due to non trivial translation
//DISABLE_TEST:SIMPLE_EX(filecheck=CHECK_CUDA): -lang glsl tests/glsl-intrinsic/shader-subgroup/shader-subgroup-builtin-variables.slang -stage compute -entry computeMain -target cuda -DTARGET_CUDA
//missing implementation of system (varying?) values
//DISABLE_TEST:SIMPLE_EX(filecheck=CHECK_CPP): -lang glsl tests/glsl-intrinsic/shader-subgroup/shader-subgroup-builtin-variables.slang -stage compute -entry computeMain -target cpp -DTARGET_CPP

//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -entry computeMain -source-language glsl -Xslang -xslang -DTARGET_VK
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -entry computeMain -source-language glsl -Xslang -emit-spirv-directly -xslang -DTARGET_VK
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -entry computeMain -source-language glsl -Xslang

// There are some issues with the Metal backend when using glsl-style syntax - this should be fixed.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -entry computeMain -source-language glsl -Xslang -xslang -DTARGET_METAL

#version 430

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[];
} outputBuffer;

layout(local_size_x = 32) in;

[shader("compute")]
void computeMain()
{
    if (gl_GlobalInvocationID.x == 3) {
        outputBuffer.data[0] = true
            && gl_SubgroupSize == 32
            && gl_SubgroupInvocationID == 3
            // These intrinsics are only available on Vulkan(SPIRV and GLSL).
#if defined(TARGET_VK)
            && gl_SubgroupID  == 0 //1 subgroup, 0 based indexing
            && gl_NumSubgroups == 1
            && gl_SubgroupEqMask == uvec4(0b1000,0,0,0)
            && gl_SubgroupGeMask == uvec4(0xFFFFFFF8,0,0,0)
            && gl_SubgroupGtMask == uvec4(0xFFFFFFF0,0,0,0)
            && gl_SubgroupLeMask == uvec4(0b1111,0,0,0)
            && gl_SubgroupLtMask == uvec4(0b111,0,0,0)
#endif
            ;
    }
    // CHECK_GLSL: void main(
    // CHECK_SPV: OpEntryPoint
    // CHECK_HLSL: void computeMain(
    // CHECK_CUDA: void computeMain(
    // CHECK_CPP: void _computeMain(
    // BUF: 1
}
