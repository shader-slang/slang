//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Pins that a `unorm`/`snorm` storage image with no `[format(...)]` infers an 8-bit SPIR-V format sized by the element's component count, with the 3-component case widening to the 4-component format.
//META: intent=characterization
//META: covers=source/slang/slang-emit-spirv.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the `unorm` / `snorm` branch of `SPIRVEmitContext::ensureTextureType`.
// When a storage image has no explicit `[format(...)]`, the emitter looks for a
// `UNormAttr` / `SNormAttr` on the element type and, if it finds one, picks an 8-bit
// normalized SPIR-V format whose channel count follows the element's component count.
// Both attributes have a four-way switch on that count, so the whole branch is only
// observable by declaring every width; this file declares all eight combinations and
// reads each texture so none is dead-stripped.
//
// The interesting row is the 3-component one. SPIR-V has no 3-channel normalized
// format, so `unorm float3` does not get its own format -- it shares `Rgba8` with the
// 4-component case (and `snorm float3` shares `Rgba8Snorm`). That is why the CHECKs
// below assert six distinct `OpTypeImage` lines for eight textures, and why a
// `CHECK-NOT` guards against a hypothetical 3-channel spelling appearing.
//
// Only SPIR-V is pinned. The GLSL path is known to disagree with this one today --
// it collapses both `unorm float4` and `snorm float4` onto `layout(rgba32f)` -- so
// pinning GLSL here would codify that disagreement; it is already recorded as
// `_meta/findings/glsl-unorm-snorm-image-format-collapsed.yaml`.
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute

RWTexture2D<unorm float> u1;
RWTexture2D<unorm float2> u2;
RWTexture2D<unorm float3> u3;
RWTexture2D<unorm float4> u4;
RWTexture2D<snorm float> s1;
RWTexture2D<snorm float2> s2;
RWTexture2D<snorm float3> s3;
RWTexture2D<snorm float4> s4;

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint2 c = tid.xy;
    outputBuffer[0] = u1[c] + u2[c].x + u3[c].x + u4[c].x
                    + s1[c] + s2[c].x + s3[c].x + s4[c].x;
}

// One 8-bit normalized format per component count; `unorm float3` and `unorm float4`
// share `Rgba8`, and likewise for the snorm pair, so eight textures yield six types.
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 R8
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 Rg8
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 Rgba8
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 R8Snorm
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 Rg8Snorm
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 Rgba8Snorm
// There is no 3-channel normalized format to fall back to.
//SPV-NOT: 2 0 0 2 Rgb8
