//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:38:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=a1c3e1c21f5e7d5f5d6098ee8202f97eec56a5706e678fd2be1b3844ebe8539e
//META: purpose=Expansion: HLSL meta-module `RWTexture1D<uint>` (integer-format variant) subscript-store lowers per target — GLSL emits a uint image (uimage1D), confirming the writable-resource lowering carries the uint sampled-type.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `RWTexture1D<uint>` exercises the integer-format variant of the
// writable 1D image. GLSL emits uimage1D; SPIR-V's image type carries
// a uint sampled-type.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute

RWTexture1D<uint> tex;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    tex[tid.x] = 7u;
}

// HLSL: RWTexture1D
// GLSL: uimage1D
// GLSL: imageStore(
