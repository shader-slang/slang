// inner-coverage.slang

//TEST:CROSS_COMPILE:-target spirv -entry main -stage fragment

[shader("fragment")]
void main(
    uint innerCoverage : SV_InnerCoverage,
    out float4 result : SV_Target)
{
    result = innerCoverage;
}
