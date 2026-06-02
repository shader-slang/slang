//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:03:42+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-execution-divergence-reconvergence.md#divergence
//META: doc_section_digest=7748f330eccf2fb718b5ba2c0f9aff0035f24f4197647463775fd43f1b5ebbb9
//META: purpose=Loop statement: SPIRV emits OpLoopMerge marking the loop merge block where all threads reconverge after the loop exits.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a loop that runs different iteration counts per thread emits SPIR-V
// structured loop control. Threads loop 2 or 3 times (divergence); CHECK asserts
// `OpLoopMerge` is present, which names the block where all threads reconverge once
// every thread has exited its last loop iteration.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<uint> output;

[numthreads(4,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Different threads loop different numbers of times: divergence
    uint numLoops = 2 + (tid.x & 1);
    uint sum = 0;
    for (uint i = 0; i < numLoops; ++i)
    {
        sum += i;
    }
    // All threads exit the loop: reconvergence at the merge block
    output[tid.x] = sum;
}

// CHECK: OpLoopMerge
