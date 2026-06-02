//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:03:42+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-execution-divergence-reconvergence.md#divergence
//META: doc_section_digest=7748f330eccf2fb718b5ba2c0f9aff0035f24f4197647463775fd43f1b5ebbb9
//META: purpose=Switch fall-through: threads in the first case group reconverge with threads in the second case group at the fall-through label (common body), visible as adjacent SPIRV blocks without OpSelectionMerge between them.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "reconvergence between threads on adjacent case label groups
// occurs on a switch case fall-through." In SPIRV, the fall-through from case 0
// to case 1 appears as a plain OpBranch (not a new OpSelectionMerge), and
// both case 0 fall-through and case 1 threads execute the shared block.
// The entire switch has one OpSelectionMerge at the outer merge point.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<uint> output;

[numthreads(4,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint result = 0;
    uint v = tid.x & 1; // 0 or 1
    switch (v)
    {
    case 0:
        result += 10;
        // fall-through — threads from case 0 reconverge with case 1 threads here
    case 1:
        result += 20;
        break;
    }
    output[tid.x] = result;
}

// Outer switch must have a single merge block (one OpSelectionMerge).
// CHECK: OpSelectionMerge
// The switch dispatch uses OpSwitch.
// CHECK: OpSwitch
