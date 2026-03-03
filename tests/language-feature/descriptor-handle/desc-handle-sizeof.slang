//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type

// LLVM/CPU backend doesn't support DescriptorHandle
//DISABLE_TEST(compute):COMPARE_COMPUTE: -llvm

// Test that sizeof/alignof of DescriptorHandle works correctly.
// DescriptorHandle is represented as uint2 (8 bytes) on most targets,
// or uint64_t (8 bytes, 8-byte aligned) with spvBindlessTextureNV.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct TestStruct
{
    float4x4 matrix;
    DescriptorHandle<Texture2D> tex;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // sizeof tests - DescriptorHandle is always 8 bytes (uint2 or uint64_t)
    outputBuffer[0] = sizeof(DescriptorHandle<Texture2D>);
    outputBuffer[1] = sizeof(Texture2D.Handle);
    outputBuffer[2] = sizeof(DescriptorHandle<SamplerState>);
    outputBuffer[3] = sizeof(DescriptorHandle<StructuredBuffer<float4>>);

    // alignof tests - alignment depends on target (4 or 8 bytes)
    // We just verify they return non-zero valid values
    outputBuffer[4] = alignof(DescriptorHandle<Texture2D>);// > 0 ? 1 : 0;
    outputBuffer[5] = alignof(Texture2D.Handle);// > 0 ? 1 : 0;
    outputBuffer[6] = alignof(DescriptorHandle<SamplerState>);// > 0 ? 1 : 0;
    outputBuffer[7] = alignof(DescriptorHandle<StructuredBuffer<float4>>);// > 0 ? 1 : 0;
}

// CHECK: 8
// CHECK: 8
// CHECK: 8
// CHECK: 8
// CHECK: {{[48]}}
// CHECK: {{[48]}}
// CHECK: {{[48]}}
// CHECK: {{[48]}}
