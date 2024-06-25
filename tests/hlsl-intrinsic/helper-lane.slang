//TEST:SIMPLE: -profile ps_6_0
//TEST:SIMPLE: -profile ps_6_6

float4 main() : SV_Target
{
    // IsHelperLane() has different implementations
    // under 6.0 <= SM < 6.6, see the following:
    // https://microsoft.github.io/DirectX-Specs/d3d/HLSL_ShaderModel6_6.html#is-helper-lane
    return IsHelperLane() ? (float4) 0.0f : (float4) 1.0f;
}
