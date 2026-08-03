//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:01:06Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#builtinoperatorexpr
//META: doc_section_digest=c1890a75acdc226411b7d61f4ee2807ae95bd2bb75bf036bb2da6f27662ff946
//META: purpose=For-loop trip-count inference reads the resolved builtin operation kind: a [ForceUnroll] loop bounded by `i < (7 % 5)` unrolls into per-iteration stores.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that trip-count inference consumes the resolved operation kind
// of the builtin comparison in a loop condition. A [ForceUnroll] loop
// whose bound is the builtin-operator expression `7 % 5` (== 2) must have
// an inferable trip count; a failed inference would reject the unroll.
// After unrolling, the second iteration's store constant 41 exists in the
// emitted HLSL (a preserved loop would only contain the `40 + i` form).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    [ForceUnroll]
    for (int i = 0; i < (7 % 5); i++)
        outputBuffer[i] = 40 + i;
    // CHECK-DAG: 40
    // CHECK-DAG: 41
}
