//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage fragment -entry fragmentMain -minimum-slang-optimization
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage fragment -entry fragmentMain

// The HLSL texture methods guard their element type with `static_assert(T is float || ...)`,
// where `T` is the texel descriptor's associated `DataType`. Minimum-optimization mode skips the
// peephole pass, so the type test must already be folded by specialization for the assertion to
// see a constant condition.
Texture2D<float4> colorTexture;
Texture2D<RGBA16F> formattedTexture;
SamplerState linearSampler;

float4 fragmentMain(float2 uv : TEXCOORD) : SV_Target
{
    // CHECK: .Sample({{.*}}.SampleLevel({{.*}}.SampleBias({{.*}}.SampleGrad(
    return colorTexture.Sample(linearSampler, uv) + colorTexture.SampleLevel(linearSampler, uv, 0) +
           formattedTexture.SampleBias(linearSampler, uv, 0) +
           formattedTexture.SampleGrad(linearSampler, uv, float2(0), float2(0));
}
