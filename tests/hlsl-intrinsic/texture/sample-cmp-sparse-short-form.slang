// The short (offset, status) forms of SampleCmpGrad and SampleCmpBias are SPIR-V-only
// overloads: they lower to sparse residency with no clamp operand, unlike the HLSL
// (offset, clamp, status) forms. DXC has no counterpart for them, so no other target runs.
//TEST:SIMPLE(filecheck=CHECK):-target spirv -stage fragment -entry fragmentMain

Texture2D<float> shadowMap2D;
SamplerComparisonState comparisonSampler;

float4 fragmentMain(float4 p: SV_Position) : SV_Target
{
    uint status;
    float shadow = shadowMap2D.SampleCmpGrad(
        comparisonSampler, p.xy, 0.5, ddx(p.xy), ddy(p.xy), int2(1, 0), status);
    shadow += shadowMap2D.SampleCmpBias(comparisonSampler, p.xy, 0.5, 0.25, int2(1, 0), status);
    // CHECK: OpImageSparseSampleDrefExplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Grad|ConstOffset %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}}
    // CHECK: OpImageSparseSampleDrefImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Bias|ConstOffset %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}}
    // CHECK-NOT: MinLod
    // CHECK: OpStore
    return float4(shadow, status, 0, 1);
}
