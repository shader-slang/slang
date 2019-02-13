//TEST_DISABLED:SIMPLE: -profile lib_6_3 -entry HitMain -stage closesthit -target spirv -o vk-structured-buffer-load.spv -D HLSL
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
    float offsfloat = gParamBlock.sbuf.Load(offs);
    RayData.PackedHitInfoA.y = offsfloat;
}
