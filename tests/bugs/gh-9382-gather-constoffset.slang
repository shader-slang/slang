//TEST:SIMPLE(filecheck=CONST_OFFSET): -target spirv-asm -entry main -stage fragment -DCONST_OFFSET
//TEST:SIMPLE(filecheck=CONST_SPLAT): -target spirv-asm -entry main -stage fragment -DCONST_SPLAT
//TEST:SIMPLE(filecheck=OFFSET): -target spirv-asm -entry main -stage fragment

// Regression test for https://github.com/shader-slang/slang/issues/9382
//
// When `OpImageGather` takes a compile-time constant value as its offset, the
// `ConstOffset` image operand is sufficient and the `ImageGatherExtended`
// capability must NOT be declared. When the offset is a runtime value,
// `ImageGatherExtended` is required and the operand must be `Offset` (not
// `ConstOffset`, which the SPIR-V spec requires to be a constant).

// CONST_OFFSET-NOT:OpCapability ImageGatherExtended
// CONST_SPLAT-NOT:OpCapability ImageGatherExtended
// OFFSET:OpCapability ImageGatherExtended

[[vk::binding(0, 0)]]
Texture2D<float> tex;

[[vk::binding(1, 0)]]
SamplerState s;

[shader("fragment")]
float4 main([[vk::location(0)]] float2 uv : UV) : SV_Target
{
#if defined(CONST_OFFSET)
    // A constant offset built from per-component literals (`MakeVector`).
    // CONST_OFFSET:OpImageGather %
    // CONST_OFFSET-SAME: ConstOffset %
    return tex.Gather(s, uv, int2(2, 1));
#elif defined(CONST_SPLAT)
    // A constant offset built by splatting a scalar (`MakeVectorFromScalar`) is
    // still compile-time constant, so it must also avoid `ImageGatherExtended`.
    // CONST_SPLAT:OpImageGather %
    // CONST_SPLAT-SAME: ConstOffset %
    return tex.Gather(s, uv, int2(1));
#else
    // A runtime (non-constant) offset must keep `Offset` (matched with a
    // leading space so it does not also match the `ConstOffset` substring).
    // OFFSET:OpImageGather %
    // OFFSET-SAME: {{ }}Offset %
    return tex.Gather(s, uv, int2(uv));
#endif
}
