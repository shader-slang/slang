//TEST:COMPARE_HLSL:-slang -dx12 -use-dxil -profile ps_6_0

float4 main() : SV_Target
{
    return IsHelperLane() ? (float4) 0.0f : (float4) 1.0f;
}
