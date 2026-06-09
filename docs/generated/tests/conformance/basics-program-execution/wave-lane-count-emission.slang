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

// Verifies that `WaveGetLaneCount()` compiles in a compute entry point and lowers
// to the correct target intrinsic. CHECK asserts HLSL emits `WaveGetLaneCount` and
// SPIR-V emits `SubgroupSize`, confirming the wave-size query is mapped to the
// target-defined built-in rather than being folded away.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-CUDA):-target cuda -entry computeMain -stage compute

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

// GLSL emits gl_SubgroupSize built-in with the subgroup_basic extension.
// CHECK-GLSL: gl_SubgroupSize

// Metal emits threads_per_simdgroup built-in parameter.
// CHECK-METAL: threads_per_simdgroup

// WGSL emits subgroup_size built-in with the subgroups enable directive.
// CHECK-WGSL: subgroup_size

// CUDA emits warpSize built-in constant.
// CHECK-CUDA: warpSize
