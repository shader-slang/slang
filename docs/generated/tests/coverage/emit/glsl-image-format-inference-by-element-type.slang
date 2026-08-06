//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Pins the GLSL format that is inferred for an `RWTexture2D` with no `[format(...)]` attribute, across component counts 1/2/3/4 and element types float/half/uint/int/int16/uint8.
//META: intent=characterization
//META: covers=source/slang/slang-emit-glsl.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the *inference* half of `GLSLSourceEmitter::_emitGLSLImageFormatModifier`,
// which runs when a storage image carries no `[format(...)]` attribute. The inferred
// name is assembled from two independent switches: the component count picks the
// channel prefix (`r` / `rg` / `rgba`) and the element's base type picks the size and
// class suffix (`32f` / `16f` / `32ui` / `32i` / `16i` / `8ui`). One texture per
// interesting combination makes both switches observable at once, and the CHECKs pair
// each inferred `layout(...)` with the declaration it precedes.
//
// The 3-component row is the reason this is worth pinning rather than assuming: GLSL
// has no 3-channel storage format, so `RWTexture2D<float3>` does *not* infer `rgb32f`
// -- it widens to `rgba32f`, sharing the arm used for 4-component textures. That
// silent widening is exactly the kind of behaviour a characterization test should hold
// still.
//
// The element type also drives `_emitGLSLTypePrefix`, so each declaration additionally
// pins the image type name (`image2D` / `uimage2D` / `iimage2D` / `i16image2D` /
// `u8image2D`), and the narrow types pull in the 8-/16-bit storage extensions.
// Every texture is written in the entry point so none is dropped before emit.
//
// SPIR-V is pinned next to it, because it runs its own inference over the same
// declarations and reaches a *different* answer, which is worth holding still. The
// striking part is the float rows: `float`, `float2` and `float3` all share a single
// `OpTypeImage ... Unknown`, so on that target neither the component count nor the
// 32-bit float element type produces a format at all, while GLSL infers `r32f` /
// `rg32f` / `rgba32f` for the same three declarations. The narrow and integer element
// types do infer on both. A `CHECK-NOT` guards the absence of any 32-bit float format
// operand, since that absence is the observation.
//
// HLSL and Metal are not pinned: neither carries a storage-image format at all --
// HLSL emits `RWTexture2D<float>` and Metal `texture2d<float, access::read_write>` --
// so there is no token for a CHECK to assert. WGSL is not pinned because it cannot
// compile these declarations: the `int16_t4` and `uint8_t4` rows fail with E41400
// ("static assertion failed") and E38203 ("disallowed vector element type").
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute

RWTexture2D<float> f1;
RWTexture2D<float2> f2;
RWTexture2D<float3> f3;
RWTexture2D<half4> h4;
RWTexture2D<uint2> u2;
RWTexture2D<int> i1;
RWTexture2D<int16_t4> s4;
RWTexture2D<uint8_t4> b4;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint2 c = tid.xy;
    f1[c] = 1.0;
    f2[c] = float2(1, 2);
    f3[c] = float3(1, 2, 3);
    h4[c] = half4(1, 2, 3, 4);
    u2[c] = uint2(1, 2);
    i1[c] = 1;
    s4[c] = int16_t4(1, 2, 3, 4);
    b4[c] = uint8_t4(1, 2, 3, 4);
}

// Narrow element types require the small-storage extensions.
//GLSL-DAG: #extension GL_EXT_shader_16bit_storage : require
//GLSL-DAG: #extension GL_EXT_shader_8bit_storage : require

// 1 component -> `r`, 2 -> `rg`, 4 -> `rgba`.
//GLSL-DAG: layout(r32f)
//GLSL-DAG: uniform image2D f1
//GLSL-DAG: layout(rg32f)
//GLSL-DAG: uniform image2D f2
// 3 components has no GLSL format and is widened to the 4-component one.
//GLSL-DAG: layout(rgba32f)
//GLSL-DAG: uniform image2D f3
// `half` selects the 16-bit float suffix.
//GLSL-DAG: layout(rgba16f)
//GLSL-DAG: uniform image2D h4
// Integer element types select the `ui` / `i` classes and the matching type prefix.
//GLSL-DAG: layout(rg32ui)
//GLSL-DAG: uniform uimage2D u2
//GLSL-DAG: layout(r32i)
//GLSL-DAG: uniform iimage2D i1
//GLSL-DAG: layout(rgba16i)
//GLSL-DAG: uniform i16image2D s4
//GLSL-DAG: layout(rgba8ui)
//GLSL-DAG: uniform u8image2D b4

// The SPIR-V side of the same declarations. Six distinct `OpTypeImage` lines cover the
// eight textures: the three float rows collapse onto one `Unknown` entry, and the five
// narrow / integer rows each infer a format.
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 Unknown
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 Rgba16f
//SPV-DAG: OpTypeImage %uint 2D 2 0 0 2 Rg32ui
//SPV-DAG: OpTypeImage %int 2D 2 0 0 2 R32i
//SPV-DAG: OpTypeImage %int 2D 2 0 0 2 Rgba16i
//SPV-DAG: OpTypeImage %uint 2D 2 0 0 2 Rgba8ui
// No 32-bit float format is inferred on this target, unlike GLSL above.
//SPV-NOT: OpTypeImage %float 2D 2 0 0 2 R32f
//SPV-NOT: OpTypeImage %float 2D 2 0 0 2 Rg32f
//SPV-NOT: OpTypeImage %float 2D 2 0 0 2 Rgba32f
