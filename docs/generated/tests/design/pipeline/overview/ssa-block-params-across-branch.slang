//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/overview.md#ast--ir-lowering
//META: doc_section_digest=d2bb343d99876dfd1d05e29a3d730266a1a5c82ccace506601773d10ac4885df
//META: purpose=Expressions lower to SSA values and a value produced on both sides of a branch is carried to the join point by a block parameter, which reaches SPIR-V as an OpPhi.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the SSA half of expression lowering: `v` is assigned different
// expressions in the two arms and read after the join, which is exactly the
// shape a block parameter exists to express. SPIR-V is the only text target
// with a surface form for it, so the CHECK pins `OpPhi %int` in the merge
// block. `-O2` is required because at the harness default of `-O0` the
// mem2reg-style promotion is skipped and the value is instead round-tripped
// through a function-scope variable, hiding the block parameter. The
// companion CHECKs assert both incoming expressions are still computed in
// their own blocks, so the phi is a real join and not a folded constant.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute -O2

RWStructuredBuffer<int> gOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int v;
    if (tid.x > 0u)
        v = int(tid.x) * 2;
    else
        v = int(tid.x) + 7;
    gOut[0] = v;
}

// SPIRV-DAG: OpSelectionMerge
// SPIRV-DAG: OpIMul %int
// SPIRV-DAG: OpIAdd %int
// SPIRV-DAG: OpPhi %int
