//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:03:42+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-execution-divergence-reconvergence.md#divergence
//META: doc_section_digest=7748f330eccf2fb718b5ba2c0f9aff0035f24f4197647463775fd43f1b5ebbb9
//META: purpose=`switch` statement: SPIRV emits OpSelectionMerge + OpSwitch encoding divergence into distinct case-label groups and reconvergence at the merge block.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `switch` with multiple case groups emits structured selection
// on every back-end. Threads fan out to case 0, 1, or default based on
// tid.x % 3; all reconverge after the switch statement.

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
    uint v = tid.x % 3;
    switch (v)
    {
    case 0:
        result = 10;
        break;
    case 1:
        result = 20;
        break;
    default:
        result = 30;
        break;
    }
    output[tid.x] = result;
}

// SPIRV: OpSelectionMerge marks reconvergence; OpSwitch dispatches to case blocks.
// CHECK: OpSelectionMerge
// CHECK: OpSwitch

// Metal: switch statement preserved with three case arms.
// CHECK_METAL: switch(
// CHECK_METAL: case 0U
// CHECK_METAL: case 1U
// CHECK_METAL: default

// WGSL: switch statement preserved with three case arms.
// CHECK_WGSL: switch(
// CHECK_WGSL: case u32(0)
// CHECK_WGSL: case u32(1)
// CHECK_WGSL: default

// CUDA: switch statement preserved with three case arms.
// CHECK_CUDA: switch(
// CHECK_CUDA: case 0U
// CHECK_CUDA: case 1U
// CHECK_CUDA: default

// C++: switch statement preserved with three case arms.
// CHECK_CPP: switch(
// CHECK_CPP: case 0U
// CHECK_CPP: case 1U
// CHECK_CPP: default
