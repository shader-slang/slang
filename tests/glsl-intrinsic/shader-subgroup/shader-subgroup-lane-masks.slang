//TEST:SIMPLE(filecheck=CHECK_GLSL):  -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage compute -entry computeMain -target spirv -emit-spirv-directly

// Test that GLSL subgroup lane mask built-in variables work on all targets via the new Wave* functions
//TEST:SIMPLE(filecheck=CHECK_HLSL): -allow-glsl -stage compute -entry computeMain -target hlsl
//TEST:SIMPLE(filecheck=CHECK_CUDA): -allow-glsl -stage compute -entry computeMain -target cuda

//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -entry computeMain -allow-glsl
// Metal disabled: lane mask computation may differ on Apple GPUs
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -entry computeMain -allow-glsl

#version 430

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    uint data[];
} outputBuffer;

layout(local_size_x = 8) in;

[shader("compute")]
void computeMain()
{
    uint laneIndex = gl_SubgroupInvocationID;
    
    // Test gl_SubgroupEqMask - should have only the current lane's bit set
    uvec4 eqMask = gl_SubgroupEqMask;
    bool eqOk = (eqMask.x == (1u << laneIndex));
    
    // Test gl_SubgroupGeMask - should have bits >= laneIndex set
    uvec4 geMask = gl_SubgroupGeMask;
    bool geOk = (geMask.x == (~0u << laneIndex));
    
    // Test gl_SubgroupGtMask - should have bits > laneIndex set
    uvec4 gtMask = gl_SubgroupGtMask;
    uint expectedGt = (laneIndex >= 31u) ? 0u : (~0u << (laneIndex + 1u));
    bool gtOk = (gtMask.x == expectedGt);
    
    // Test gl_SubgroupLeMask - should have bits <= laneIndex set
    uvec4 leMask = gl_SubgroupLeMask;
    uint expectedLe = (laneIndex >= 31u) ? ~0u : ((1u << (laneIndex + 1u)) - 1u);
    bool leOk = (leMask.x == expectedLe);
    
    // Test gl_SubgroupLtMask - should have bits < laneIndex set
    uvec4 ltMask = gl_SubgroupLtMask;
    bool ltOk = (ltMask.x == ((1u << laneIndex) - 1u));
    
    if (gl_GlobalInvocationID.x == 3)
    {
        outputBuffer.data[0] = (eqOk && geOk && gtOk && leOk && ltOk) ? 1u : 0u;
    }
    
    // CHECK_GLSL: void main(
    // CHECK_SPV: OpEntryPoint
    // CHECK_HLSL: void computeMain(
    // CHECK_CUDA: void computeMain(
    // BUF: 1
}

