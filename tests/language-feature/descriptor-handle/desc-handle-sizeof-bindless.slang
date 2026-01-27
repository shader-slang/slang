//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability spvBindlessTextureNV -stage compute -entry computeMain

// Test that sizeof/alignof of DescriptorHandle with spvBindlessTextureNV capability
// returns correct values. With spvBindlessTextureNV, DescriptorHandle is uint64_t
// (8 bytes, 8-byte aligned) instead of uint2.

RWStructuredBuffer<int> outputBuffer;

struct TestStruct
{
    float4x4 matrix;
    DescriptorHandle<Texture2D> tex;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // sizeof should be 8 (uint64_t)
    outputBuffer[0] = sizeof(DescriptorHandle<Texture2D>);
    outputBuffer[1] = sizeof(Texture2D.Handle);
    outputBuffer[2] = sizeof(DescriptorHandle<SamplerState>);

    // alignof should be 8 (uint64_t alignment)
    outputBuffer[3] = alignof(DescriptorHandle<Texture2D>);
    outputBuffer[4] = alignof(Texture2D.Handle);
    outputBuffer[5] = alignof(DescriptorHandle<SamplerState>);

    // TestStruct should have proper size accounting for uint64_t alignment
    outputBuffer[6] = sizeof(TestStruct);
    outputBuffer[7] = alignof(TestStruct);
}

// Verify constants are defined (order-independent)
// CHECK-DAG: %int_8 = OpConstant %int 8

// Verify sizeof and alignof values are stored as 8
// The first 6 stores should all be int_8 (sizeof=8, alignof=8 for DescriptorHandle)
// CHECK: OpStore {{%[0-9]+}} %int_8
// CHECK: OpStore {{%[0-9]+}} %int_8
// CHECK: OpStore {{%[0-9]+}} %int_8
// CHECK: OpStore {{%[0-9]+}} %int_8
// CHECK: OpStore {{%[0-9]+}} %int_8
// CHECK: OpStore {{%[0-9]+}} %int_8
