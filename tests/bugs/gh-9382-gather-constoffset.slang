//TEST:SIMPLE(filecheck=CONST_OFFSET): -target spirv-asm -entry main -stage fragment -DCONST_OFFSET
//TEST:SIMPLE(filecheck=OFFSET): -target spirv-asm -entry main -stage fragment

// When `OpImageGather` takes a compile-time constant value as its `offset` value,
// it is enough to use a flag `ConstOffset` and it shouldn't use
// `ImageGatherExtended` capability unnecessarilly.
// When the `offset` value is a runtime value, `ImageGatherExtended` is required,
// and the flag must be `Offset` instead of `ConstOffset`.

// CONST_OFFSET-NOT:OpCapability ImageGatherExtended
// OFFSET:OpCapability ImageGatherExtended

[[vk::binding(0, 0)]]
Texture2D<float> tex;

[[vk::binding(1, 0)]]
SamplerState s;

[shader("fragment")]
float4 main([[vk::location(0)]] float2 uv : UV) : SV_Target
{
#if defined(CONST_OFFSET)
    // CONST_OFFSET:OpImageGather %
    // CONST_OFFSET-SAME: ConstOffset %
    return tex.Gather(s, uv, int2(2, 1));
#else
    // OFFSET:OpImageGather %
    // OFFSET-SAME: {{ }}Offset %
    return tex.Gather(s, uv, int2(uv));
#endif
}
