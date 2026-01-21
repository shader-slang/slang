//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute

// Test that sizeof/alignof of DescriptorHandle produces an error because
// the size is target-dependent (uint2 vs uint64_t depending on target).
// Users should use reflection API to query size at runtime instead.

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // sizeof tests
    // CHECK: error 30100
    outputBuffer[0] = sizeof(DescriptorHandle<Texture2D>);

    // CHECK: error 30100
    outputBuffer[1] = sizeof(Texture2D.Handle);

    // CHECK: error 30100
    outputBuffer[2] = sizeof(DescriptorHandle<SamplerState>);

    // CHECK: error 30100
    outputBuffer[3] = sizeof(DescriptorHandle<StructuredBuffer<float4>>);

    // alignof tests
    // CHECK: error 30100
    outputBuffer[4] = alignof(DescriptorHandle<Texture2D>);

    // CHECK: error 30100
    outputBuffer[5] = alignof(Texture2D.Handle);

    // CHECK: error 30100
    outputBuffer[6] = alignof(DescriptorHandle<SamplerState>);

    // CHECK: error 30100
    outputBuffer[7] = alignof(DescriptorHandle<StructuredBuffer<float4>>);
}
