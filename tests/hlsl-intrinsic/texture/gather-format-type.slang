//TEST:SIMPLE: -target hlsl -stage fragment -entry fragmentMain -o gather-format-type.hlsl
//TEST:SIMPLE: -target spirv -stage fragment -entry fragmentMain -o gather-format-type.spv

// The four-offset gather helper must receive the texel descriptor, while its
// result contains the descriptor's scalar payload elements.
Texture2D<RGBA16F> texture;
SamplerState sampler;

float4 fragmentMain(float2 uv : TEXCOORD) : SV_Target
{
    return texture.GatherGreen(sampler, uv, int2(0, 0), int2(1, 0), int2(0, 1), int2(1, 1));
}
