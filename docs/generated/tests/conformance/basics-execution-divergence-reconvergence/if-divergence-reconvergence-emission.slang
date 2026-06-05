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

// Verifies that an `if`/`else` producing divergence emits structured control
// flow for reconvergence across multiple back-ends. The shader branches on an
// odd/even condition. Emission tests pin the back-end-specific constructs that
// encode the divergence point and the two arms.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_CPP):-target cpp -entry computeMain -stage compute

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

// SPIRV: structured selection with merge block and conditional branch.
// CHECK: OpSelectionMerge
// CHECK: OpBranchConditional

// Metal: if/else preserved with the two arms.
// CHECK_METAL: if(
// CHECK_METAL: result_0 = 1U
// CHECK_METAL: result_0 = 2U

// WGSL: if/else preserved; both arms visible.
// CHECK_WGSL: if(
// CHECK_WGSL: result_0 = u32(1)
// CHECK_WGSL: result_0 = u32(2)

// CUDA: if/else preserved; both arms visible.
// CHECK_CUDA: if(
// CHECK_CUDA: result_0 = 1U
// CHECK_CUDA: result_0 = 2U

// C++: if/else preserved; both arms visible.
// CHECK_CPP: if(
// CHECK_CPP: result_0 = 1U
// CHECK_CPP: result_0 = 2U
