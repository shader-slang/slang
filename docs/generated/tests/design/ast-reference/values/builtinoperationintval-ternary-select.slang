//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:00:25Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/values.md#builtinoperationintval-and-the-single-representation-invariant
//META: doc_section_digest=4229b55a01470c7ad99014966f3313e48dd3d9bca875bef59b0f7b4c86cbc182
//META: purpose=A conditional operator ?: over generic value operands is a symbolic compile-time integer that folds to the selected branch after substitution; each instantiation carries the branch its condition picks.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `?:` (a resolved-operator-call form of the symbolic
// builtin operation) participates in a compile-time array bound. The
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
