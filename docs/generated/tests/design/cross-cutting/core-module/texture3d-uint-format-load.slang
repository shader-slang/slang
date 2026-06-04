//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:38:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=120aee6df2c1b3d3ea42ea63294c00830c98fd3053f546fc49ad4100f4318f68
//META: purpose=Expansion: HLSL meta-module `Texture3D<uint4>` (integer-format variant) lowers per target — GLSL emits a uint-flavoured sampler (utexture3D/usampler3D), confirming the format generic propagates through the resource lowering.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The HLSL-meta-module's `Texture3D<T>` is parameterised on the texel
// format. With `uint4`, GLSL must emit a uint-flavoured sampler type
// and the SPIR-V image sampled-type must be a uint.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute

Texture3D<uint4> tex;
RWStructuredBuffer<uint4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.Load(int4(int(tid.x), int(tid.y), int(tid.z), 0));
}

// HLSL: Texture3D
// HLSL: Load(
// GLSL: {{(utexture3D|usampler3D)}}
