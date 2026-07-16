//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:06:56+00:00
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#auto-generated-reference
//META: doc_section_digest=8ae3fbfa6c36eddd14d01848a2078f5e7d7dcff82bbdca7e808de3783b980183
//META: purpose=The `subgroup_workgroup_index` compound alias admits `WaveGetWaveIndex` / `WaveGetNumWaves` in a compute entry point; the queries compile and surface subgroup constructs in the emitted GLSL and SPIR-V assembly.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Positive companion to the fragment-rejection probes: the same wave-index
// queries the compound alias restricts to compute-class stages must be
// accepted from a compute entry point on GLSL and SPIR-V. The results are
// written to a buffer so DCE cannot strip them; each CHECK asserts the
// emitted text contains a subgroup construct, proving the queries passed
// the capability gate and lowered on both targets the doc names.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = WaveGetWaveIndex() + WaveGetNumWaves();
}

// GLSL: {{[Ss]ubgroup}}
// SPIRV: {{Subgroup|GroupNonUniform}}
