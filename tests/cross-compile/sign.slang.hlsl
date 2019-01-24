//TEST_IGNORE_FILE:
float4 main() : SV_Target
{
    float4 s = sign(float4(1.5, 1.0, -1.5, -1.0));
    return s;
}