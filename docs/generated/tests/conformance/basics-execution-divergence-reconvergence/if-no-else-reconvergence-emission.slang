//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:03:42+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-execution-divergence-reconvergence.md#divergence
//META: doc_section_digest=7748f330eccf2fb718b5ba2c0f9aff0035f24f4197647463775fd43f1b5ebbb9
//META: purpose=`if` without `else`: reconvergence still occurs at the if-exit point; SPIRV emits OpSelectionMerge pointing to the single merge block after the then-arm.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Example 2 shows a plain `if (cond) { ... }` without an else clause.
// Reconvergence occurs when threads exit the `then` branch, at the point
// immediately after the if statement. Each back-end must encode both paths
// (then-arm and implicit skip) and converge to a single continuation.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<uint> output;

[numthreads(4,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint result = 5;
    bool cond = (tid.x & 1) == 0;
    if (cond)
    {
        result = 1; // only even threads reach here
    }
    // reconvergence: all threads continue here
    output[tid.x] = result;
}

// SPIRV: both paths (then + skip) converge at the merge block.
// CHECK: OpSelectionMerge
// CHECK: OpBranchConditional

// Metal: single if, both initializer value and then-arm value present.
// CHECK_METAL: if(
// CHECK_METAL: result_0 = 1U

// WGSL: single if, both initializer value and then-arm value present.
// CHECK_WGSL: if(
// CHECK_WGSL: result_0 = u32(1)

// CUDA: single if, both initializer value and then-arm value present.
// CHECK_CUDA: if(
// CHECK_CUDA: result_0 = 1U

// C++: single if, both initializer value and then-arm value present.
// CHECK_CPP: if(
// CHECK_CPP: result_0 = 1U
