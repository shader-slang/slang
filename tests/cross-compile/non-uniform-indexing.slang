//TEST:CROSS_COMPILE:-target spirv-assembly -entry main -stage fragment
//TEST:CROSS_COMPILE:-target spirv-assembly -entry main -stage fragment -verify-debug-serial-ir

// Confirm that `NonUniformResourceIndex` translates to SPIR-V as expected

Texture2D t[10];
SamplerState s;

float4 main(
    float3 uv : UV)
    : SV_Target
{
    return t[NonUniformResourceIndex(int(uv.z))].Sample(s, uv.xy);
}
