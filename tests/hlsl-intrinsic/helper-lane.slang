//TEST:SIMPLE: -profile ps_6_0
//TEST:SIMPLE: -profile ps_6_6

float4 main() : SV_Target
{
    return IsHelperLane() ? (float4) 0.0f : (float4) 1.0f;
}
