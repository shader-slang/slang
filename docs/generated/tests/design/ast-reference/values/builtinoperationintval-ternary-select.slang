//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#builtinoperationintval-and-the-single-representation-invariant
//META: doc_section_digest=7294e983132681667c88eb7937aa63061a1f9c975bb9567ce8dd36184dcda775
//META: purpose=The conditional operator is one of the resolved-operator-call forms that reaches the symbolic builtin-operation value, so a ?: over a generic value parameter folds to the branch its condition selects.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `?:` participates in a compile-time array bound. The
// parameter type int[(N > 4) ? 4 : N] is instantiated twice: N = 7
// must select the true branch (bound 4) and N = 2 the false branch
// (bound 2); each call type-checks only against the folded branch
// value, and CHECK asserts both runtime sums.

//TEST:INTERPRET(filecheck=CHECK):

int pickSum<let N : int>(int a[(N > 4) ? 4 : N])
{
    int total = 0;
    for (int i = 0; i < ((N > 4) ? 4 : N); ++i) { total += a[i]; }
    return total;
}

void main()
{
    // N = 7: 7 > 4, so the bound folds to the true branch 4.
    int four[4] = { 1, 2, 3, 4 };
    // CHECK: big=10
    printf("big=%d\n", pickSum<7>(four));

    // N = 2: 2 > 4 is false, so the bound folds to N itself, 2.
    int two[2] = { 5, 6 };
    // CHECK: small=11
    printf("small=%d\n", pickSum<2>(two));
}
