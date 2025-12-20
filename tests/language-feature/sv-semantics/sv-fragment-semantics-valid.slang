//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// CHECK: OpEntryPoint

// Test fragment-only semantics work correctly
// SV_Position as input in fragment should work (it's gl_FragCoord)

[shader("fragment")]
float4 main(
    float2 pointCoord : SV_PointCoord,
    uint sampleIndex : SV_SampleIndex,
    bool isFrontFace : SV_IsFrontFace,
    int layer : SV_RenderTargetArrayIndex,
    int viewport : SV_ViewportArrayIndex,
    uint primID : SV_PrimitiveID,
    float4 fragCoord : SV_Position)  // This should be valid as input
    : SV_Target
{
    return float4(pointCoord, fragCoord.z, 1);
}

