//TEST:REFLECTION:-stage compute -entry computeMain -target cuda -no-codegen
//TEST:REFLECTION:-stage compute -entry computeMain -target metallib -no-codegen

// Test for bindless targets, DescriptorHandle should decompose to uint64_t handle pointers
// This complements gh-6657.slang which tests SPIR-V decomposition to uint2

struct BindlessData
{
    float4 data1;
    DescriptorHandle<Texture2D> textureHandle;
    DescriptorHandle<RWStructuredBuffer<float4>> bufferHandle;
    float4 data2;
}

[numthreads(1,1,1)]
void computeMain(BindlessData bindlessStruct)
{
    // Simple usage to avoid dead code elimination
}
