//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#option-set-toggles
//META: doc_section_digest=7c951a88b67c0d94a6d4504f2d7616fdfe324b58f96f2245c67178d693892a88
//META: purpose=The three trace-coverage toggles do not gate a pre-link pass; they change what per-decl lowering emits, adding line, function-entry, and per-branch-arm counters.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "influences per-decl lowering rather than gating a pass"
// characterisation of the three trace toggles. The same shader is compiled
// four times; with no toggle the snapshot carries no counter instruction
// of any kind, and each toggle adds exactly its own counter flavour to the
// lowered function. Because the counters are already present in the
// pre-link snapshot, they must have come from the declaration walk in
// Phase A rather than from a later pass.

//TEST:SIMPLE(filecheck=NONE):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=LINE):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -trace-coverage
//TEST:SIMPLE(filecheck=FUNC):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -trace-function-coverage
//TEST:SIMPLE(filecheck=BRANCH):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -trace-branch-coverage

uniform int a;
uniform RWStructuredBuffer<int> buf;

int helper(int x) { return x + 1; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (a > 0) { buf[0] = helper(a); }
    else { buf[0] = 0; }
}

// NONE-LABEL: ### LOWER-TO-IR:
// NONE-NOT: IncrementCoverageCounter
// NONE-NOT: IncrementFunctionCoverageCounter
// NONE-NOT: IncrementBranchCoverageCounter

// LINE-LABEL: ### LOWER-TO-IR:
// LINE: IncrementCoverageCounter

// FUNC-LABEL: ### LOWER-TO-IR:
// FUNC: IncrementFunctionCoverageCounter(

// BRANCH-LABEL: ### LOWER-TO-IR:
// BRANCH: IncrementBranchCoverageCounter(
