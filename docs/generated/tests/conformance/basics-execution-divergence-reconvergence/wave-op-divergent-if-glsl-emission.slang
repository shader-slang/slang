//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:03:42+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-execution-divergence-reconvergence.md#divergence
//META: doc_section_digest=7748f330eccf2fb718b5ba2c0f9aff0035f24f4197647463775fd43f1b5ebbb9
//META: purpose=Wave-tangled ops (WaveActiveMin/WaveActiveMax) inside a divergent `if` emit as GLSL subgroup ops (subgroupMin/subgroupMax) inside their respective if-arms, requiring the GL_KHR_shader_subgroup_arithmetic extension.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that wave ops inside a divergent `if` lower to back-end-specific
// subgroup intrinsics. For targets requiring explicit feature opt-in (GLSL,
// WGSL) the emitted code must include the relevant extension/enable directive.
// Metal and CUDA include subgroup support via a prelude header — no explicit
// extension declaration appears in their output.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_CUDA):-target cuda -entry computeMain -stage compute

RWStructuredBuffer<uint> output;

[numthreads(4,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint result = 0;
    if ((tid.x & 1) == 0)
    {
        result = WaveActiveMin(tid.x);
    }
    else
    {
        result = WaveActiveMax(tid.x);
    }
    output[tid.x] = result;
}

// GLSL: explicit extension required; subgroupMin/Max are the lowered intrinsics.
// CHECK: GL_KHR_shader_subgroup_arithmetic
// CHECK: subgroupMin
// CHECK: subgroupMax

// WGSL: `enable subgroups;` is the WGSL equivalent of GLSL's extension pragma.
// CHECK_WGSL: enable subgroups
// CHECK_WGSL: subgroupMin
// CHECK_WGSL: subgroupMax

// Metal: simd_min/simd_max are the Metal equivalents; no extension declaration needed.
// CHECK_METAL: simd_min
// CHECK_METAL: simd_max

// CUDA: prelude-provided _waveMin/_waveMax helpers; no extension declaration needed.
// CHECK_CUDA: WaveActiveMin_0
// CHECK_CUDA: WaveActiveMax_0
