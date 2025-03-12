//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -O0 -g3

RWStructuredBuffer<float4> outputBuffer;

// Function that calls max and normalize three times each
[ForceInline]
float3 multipleVectorOps(float3 input)
{
    // First set of operations
    float3 maxResult1 = max(input, float3(1.0, -1.0, 0.0));
    float3 normResult1 = normalize(maxResult1);

    // Second set of operations
    float3 maxResult2 = max(normResult1, float3(0.5, 0.5, 0.5));
    float3 normResult2 = normalize(maxResult2);

    // Third set of operations
    float3 maxResult3 = max(normResult2, float3(-0.5, 0.0, 1.0));
    float3 normResult3 = normalize(maxResult3);

    return normResult3;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint i = dispatchThreadID.x;
    
    // Create test vector
    float3 vec = float3(i - 1.0, i + 1.0, i);
    
    // Call multipleVectorOps
    float3 result = multipleVectorOps(vec);
    
    // Store result
    outputBuffer[i] = float4(result, 1.0);
}
// We should have only 1 DebugInlinedAt coming from multipleVectorOps. The max, normalize
// should not create any.
// CHECK-COUNT-1: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}}