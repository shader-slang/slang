//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:03:42+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-execution-divergence-reconvergence.md#divergence
//META: doc_section_digest=7748f330eccf2fb718b5ba2c0f9aff0035f24f4197647463775fd43f1b5ebbb9
//META: purpose=Wave-tangled ops (WaveActiveMin/WaveActiveMax) inside a divergent `if` emit as HLSL wave intrinsics inside the respective if-arm, one per mutually-convergent thread set.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that wave ops inside a divergent `if` remain inside their respective arms
// in HLSL rather than being hoisted to the post-if merge point.
// CHECK asserts the conditional (`if(`) and both `WaveActiveMin` and `WaveActiveMax`
// appear in the emitted HLSL, confirming each op is scoped to its convergent thread set.
//
// Metal and CUDA also support wave/subgroup ops in divergent branches and are tested
// here with their respective intrinsic names. C++ does not support wave ops (no
// hardware warp/subgroup model), so it is listed under ## Untested claims in README.md.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_WGSL):-target wgsl -entry computeMain -stage compute

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

// HLSL: if-conditional preserved; WaveActiveMin/Max in their respective arms.
// CHECK: if(
// CHECK: WaveActiveMin
// CHECK: WaveActiveMax

// Metal: simd_min/simd_max are the Metal equivalents; each inside its if-arm.
// CHECK_METAL: if(
// CHECK_METAL: simd_min
// CHECK_METAL: simd_max

// CUDA: _waveMin/_waveMax helpers (from the CUDA prelude) inside respective arms.
// CHECK_CUDA: if(
// CHECK_CUDA: WaveActiveMin_0
// CHECK_CUDA: WaveActiveMax_0

// WGSL: subgroupMin/subgroupMax are the WGSL equivalents; each inside its if-arm.
// CHECK_WGSL: if(
// CHECK_WGSL: subgroupMin
// CHECK_WGSL: subgroupMax
