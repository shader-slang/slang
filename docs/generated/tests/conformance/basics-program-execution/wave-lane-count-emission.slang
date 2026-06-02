//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:45+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-program-execution.md#program-execution
//META: doc_section_digest=adbd230383e252b1f6ddca986456ca84146e424a9f5eea3bdd18df8bd52b8168
//META: purpose=WaveGetLaneCount() is available in a compute entry point; it emits as WaveGetLaneCount in HLSL and as the SubgroupSize built-in in SPIR-V (wave size is target-defined per the doc).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "In both graphics launches and compute dispatches, individual
// invocations are grouped into waves. The wave size is a power-of-two in the
// range [4, 128] and is defined by the target."
//
// WaveGetLaneCount() queries the wave (subgroup) size at runtime. We verify
// that the intrinsic compiles and emits the correct target-specific representation.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-SPV):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<uint> output;

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = WaveGetLaneCount();
}

// HLSL emits WaveGetLaneCount intrinsic.
// CHECK: WaveGetLaneCount

// SPIR-V emits SubgroupSize built-in (the SPIR-V representation of wave/subgroup size).
// CHECK-SPV: SubgroupSize
