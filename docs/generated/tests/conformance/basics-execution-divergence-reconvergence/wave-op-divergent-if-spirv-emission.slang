//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:03:42+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-execution-divergence-reconvergence.md#divergence
//META: doc_section_digest=7748f330eccf2fb718b5ba2c0f9aff0035f24f4197647463775fd43f1b5ebbb9
//META: purpose=Wave-tangled ops (WaveActiveMin/WaveActiveMax) inside a divergent `if` emit as GroupNonUniform ops inside separate SPIRV branch arms, applying only to mutually-convergent threads in each arm.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that wave ops inside a divergent `if` appear in the per-arm basic blocks,
// not hoisted to the post-merge block, across SPIRV and other GPU-capable back-ends.
// SPIRV pins GroupNonUniform capability + OpSelectionMerge + per-arm ops.
// Metal, CUDA, and WGSL confirm the if structure is preserved with wave intrinsics
// inside the respective arms.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
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
        // mutually-convergent set: even threads only
        result = WaveActiveMin(tid.x);
    }
    else
    {
        // mutually-convergent set: odd threads only
        result = WaveActiveMax(tid.x);
    }
    output[tid.x] = result;
}

// SPIRV: GroupNonUniformArithmetic capability; OpSelectionMerge for the if; per-arm ops.
// CHECK: OpCapability GroupNonUniformArithmetic
// CHECK: OpSelectionMerge
// CHECK: OpGroupNonUniformUMin
// CHECK: OpGroupNonUniformUMax

// Metal: if preserved; simd_min in the then-arm, simd_max in the else-arm.
// CHECK_METAL: if(
// CHECK_METAL: simd_min
// CHECK_METAL: simd_max

// CUDA: if preserved; prelude WaveActiveMin_0/WaveActiveMax_0 in their arms.
// CHECK_CUDA: if(
// CHECK_CUDA: WaveActiveMin_0
// CHECK_CUDA: WaveActiveMax_0

// WGSL: if preserved; subgroupMin in the then-arm, subgroupMax in the else-arm.
// CHECK_WGSL: if(
// CHECK_WGSL: subgroupMin
// CHECK_WGSL: subgroupMax
