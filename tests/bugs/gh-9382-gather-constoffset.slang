// Tests for shader-slang/slang#9382: a `Gather` with a compile-time-constant offset must emit the
// SPIR-V `ConstOffset` image operand (which needs no capability), not `Offset` (which requires
// `OpCapability ImageGatherExtended`). A runtime offset must still use `Offset` + the capability.
//
// The entry points are compiled at -O0 so the operand is checked as emitted, before spirv-opt could
// fold `Offset`+constant into `ConstOffset` — the -O0 state is where the capability was previously
// over-declared (#9382) and where an unconditional `ConstOffset` would be invalid for a runtime
// offset (#5339). "Offset" is a substring of "ConstOffset", so the runtime check anchors on a
// leading space (` Offset %`) to avoid matching the constant form.

//TEST:SIMPLE(filecheck=CONST): -target spirv-asm -entry constOffset -stage fragment -O0
//TEST:SIMPLE(filecheck=SPLAT): -target spirv-asm -entry constSplat -stage fragment -O0
//TEST:SIMPLE(filecheck=RUNTIME): -target spirv-asm -entry runtimeOffset -stage fragment -O0

uniform Texture2D<float> tex;
uniform SamplerState samp;

uniform float2 coord;
uniform int2 rtOffset;

// A constant vector offset -> ConstOffset, and no ImageGatherExtended capability. The -NOT is placed
// before the OpImageGather match, so its scope covers the capabilities section at the top of the
// module where an over-declared capability would appear.
// CONST-NOT: OpCapability ImageGatherExtended
// CONST: OpImageGather{{.*}} ConstOffset
float4 constOffset() : SV_Target
{
    return tex.Gather(samp, coord, int2(2, 1));
}

// A constant splat offset (MakeVectorFromScalar) -> ConstOffset, still no capability.
// SPLAT-NOT: OpCapability ImageGatherExtended
// SPLAT: OpImageGather{{.*}} ConstOffset
float4 constSplat() : SV_Target
{
    return tex.Gather(samp, coord, int2(1));
}

// A runtime offset (from a uniform) -> Offset, which requires ImageGatherExtended.
// RUNTIME: OpCapability ImageGatherExtended
// RUNTIME: OpImageGather{{.*}} Offset %
float4 runtimeOffset() : SV_Target
{
    return tex.Gather(samp, coord, rtOffset);
}
