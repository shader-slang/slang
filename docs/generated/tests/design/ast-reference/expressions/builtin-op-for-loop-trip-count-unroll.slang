//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#builtinoperatorexpr
//META: doc_section_digest=81b1b9ef9946750fa4993fc9e38d2ffd230f962a9583bad2803017f58f335d29
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
