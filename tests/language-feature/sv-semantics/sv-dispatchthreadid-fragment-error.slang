//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// CHECK: error 30702

// Test that SV_DispatchThreadID is rejected in fragment shader (compute/mesh/amplification-only)

[shader("fragment")]
float4 main(
    uint3 threadID : SV_DispatchThreadID)
    : SV_Target
{
    return float4(threadID, 1);
}


