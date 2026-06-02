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

// The doc states: in a `switch` statement, divergence occurs when threads
// jump to different case groups; reconvergence occurs when threads exit the
// switch. SPIRV encodes this as: OpSelectionMerge <merge-block> None,
// followed by OpSwitch with one branch per case-label group.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

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

// CHECK: OpSelectionMerge
// CHECK: OpSwitch
