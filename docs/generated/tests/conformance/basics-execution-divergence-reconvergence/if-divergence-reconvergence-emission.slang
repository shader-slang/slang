//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:03:42+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-execution-divergence-reconvergence.md#divergence
//META: doc_section_digest=7748f330eccf2fb718b5ba2c0f9aff0035f24f4197647463775fd43f1b5ebbb9
//META: purpose=`if` statement: SPIRV emits OpSelectionMerge/OpBranchConditional marking divergence point and a single merge block for reconvergence.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an `if`/`else` producing divergence emits SPIR-V structured control
// flow for reconvergence. The shader branches on an odd/even condition; CHECK asserts
// SPIR-V contains `OpSelectionMerge` (naming the reconvergence merge block) and
// `OpBranchConditional` (encoding the two diverging arms).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<uint> output;

[numthreads(4,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint result = 0;
    bool cond = (tid.x & 1) == 0;
    if (cond)
    {
        result = 1;
    }
    else
    {
        result = 2;
    }
    output[tid.x] = result;
}

// CHECK: OpSelectionMerge
// CHECK: OpBranchConditional
