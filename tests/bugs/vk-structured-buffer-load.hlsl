//TEST:CROSS_COMPILE: -profile lib_6_3 -entry HitMain -stage closesthit -target spirv-assembly

#define USE_RCP 0

struct ParameterBlockTest
{
    SamplerState sam;
    StructuredBuffer<float> sbuf;
};

ConstantBuffer<ParameterBlockTest> gParamBlock;

struct RayHitInfoPacked
{
    float4 PackedHitInfoA : PACKED_HIT_INFO_A;
};

[shader("closesthit")]
void HitMain(inout RayHitInfoPacked RayData, BuiltInTriangleIntersectionAttributes Attributes)
{
    float HitT = RayTCurrent();
    RayData.PackedHitInfoA.x = HitT;
    uint offs = 0;
    uint use_rcp = USE_RCP;
    float offsfloat = gParamBlock.sbuf.Load(offs);

    use_rcp |= HitT > 0.0;

    if (use_rcp)
        RayData.PackedHitInfoA.y = rcp(offsfloat);
    else
        RayData.PackedHitInfoA.y = rsqrt(offsfloat);
}
