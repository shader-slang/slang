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

// The doc states: when wave-tangled functions are supported on divergent paths,
// they apply only between the mutually convergent thread set. Each arm of the
// divergent `if` has its own set of convergent threads. In SPIRV this appears
// as GroupNonUniform ops (OpGroupNonUniformUMin, OpGroupNonUniformUMax)
// placed inside distinct SPIRV basic blocks (the `then` and `else` arms),
// not in the merge block where all threads have reconverged.
// Requires GroupNonUniformArithmetic capability.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

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

// Wave ops require the GroupNonUniformArithmetic capability in SPIRV.
// CHECK: OpCapability GroupNonUniformArithmetic
// The if-divergence emits OpSelectionMerge + OpBranchConditional.
// CHECK: OpSelectionMerge
// Each arm contains its own GroupNonUniform reduction op.
// CHECK: OpGroupNonUniformUMin
// CHECK: OpGroupNonUniformUMax
