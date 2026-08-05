//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes
//META: doc_section_digest=9e5354e38bb09fc16bac90544687941aec8ef947b56cd9f9b65ce3ddb041f315
//META: purpose=An atomic operation on a texture element forms a pointer to the image texel first, which SPIR-V renders as `OpImageTexelPointer` feeding the atomic.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `ImageTexelPointer` row's note that the opcode "forms a
// pointer to an image texel for atomic operations". The opcode is
// synthesized inside SPIR-V target legalization, so it is not in the
// pre-legalization `-dump-ir` snapshot; the observable form is the emitted
// SPIR-V. `InterlockedAdd` on `tex[tid.xy]` is the surface, and the CHECKs
// assert an `OpImageTexelPointer` producing a `_ptr_Image_uint` and an
// `OpAtomicIAdd` consuming a pointer — the atomic cannot address a texel
// any other way, which is the reason the opcode exists.
//
// The other shader-text targets are the contrast that shows the texel pointer
// is a SPIR-V-specific step rather than something in the source: GLSL and MSL
// both have a texture-atomic call that takes the coordinate directly, so the
// same `InterlockedAdd(tex[tid.xy], ...)` emits `imageAtomicAdd(tex, ivec2(..))`
// on GLSL and `tex.atomic_fetch_add(coord, ..)` on Metal, with no intermediate
// pointer value in either. WGSL has no texture-atomic at all — the entry point
// is rejected with E36107 "unavailable features in entry point" naming
// `InterlockedAdd` — so WGSL is recorded as `unsupported-on-target` in the
// bundle README instead of being given a directive.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main

uniform RWTexture2D<uint> tex;
uniform RWStructuredBuffer<uint> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint orig;
    InterlockedAdd(tex[tid.xy], 1u, orig);
    outbuf[0] = orig;
}

// SPV: %{{[A-Za-z0-9_]+}} = OpImageTexelPointer %_ptr_Image_uint %tex %{{[A-Za-z0-9_]+}}
// SPV: OpAtomicIAdd %uint

// HLSL: InterlockedAdd(tex_{{[0-9]+}}[

// GLSL: imageAtomicAdd(tex_{{[0-9]+}}, ivec2(

// METAL: tex_{{[0-9]+}}.atomic_fetch_add(tid_{{[0-9]+}}.xy,
