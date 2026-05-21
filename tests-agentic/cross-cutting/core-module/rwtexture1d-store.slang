//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:38:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#core-module
//META: doc_section_digest=120aee6df2c1b3d3ea42ea63294c00830c98fd3053f546fc49ad4100f4318f68
//META: purpose=Expansion: HLSL meta-module `RWTexture1D[idx] = value` (subscript-store) lowers per target (HLSL subscript-store, GLSL imageStore over an image1D, SPIR-V OpImageWrite).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `RWTexture1D<T>[idx] = value` writes a texel through the subscript
// operator. GLSL/SPIR-V emit explicit imageStore / OpImageWrite.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

RWTexture1D<float4> tex;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    tex[tid.x] = float4(1.0, 0.0, 0.0, 1.0);
}

// HLSL: RWTexture1D
// HLSL: tex{{.*}}[
// GLSL: imageStore(
// SPIRV: OpImageWrite
