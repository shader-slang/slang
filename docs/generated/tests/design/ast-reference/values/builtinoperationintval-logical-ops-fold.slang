//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:00:25Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/values.md#builtinoperationintval-and-the-single-representation-invariant
//META: doc_section_digest=4229b55a01470c7ad99014966f3313e48dd3d9bca875bef59b0f7b4c86cbc182
//META: purpose=Logical && and || over comparisons of a generic value parameter fold with the enclosing ?: to a concrete array bound after substitution.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the short-circuit operators (resolved-operator-call
// forms of the symbolic builtin operation) evaluate at compile time
// once their generic operands are substituted. gateAnd's bound selects
// 3 only when both comparisons hold; gateOr's selects 1 only when
// either holds. Each call type-checks against the folded bound, and
// CHECK asserts the sums from both branches of both operators.

//TEST:INTERPRET(filecheck=CHECK):

int gateAnd<let N : int>(int a[(N > 0 && N < 10) ? 3 : 1])
{
    int total = 0;
    for (int i = 0; i < ((N > 0 && N < 10) ? 3 : 1); ++i) { total += a[i]; }
    return total;
}

int gateOr<let N : int>(int a[(N < 0 || N > 10) ? 1 : 3])
{
    int total = 0;
    for (int i = 0; i < ((N < 0 || N > 10) ? 1 : 3); ++i) { total += a[i]; }
    return total;
}

void main()
{
    int three[3] = { 1, 2, 3 };
    int one[1] = { 9 };

    // N = 5: both 5 > 0 and 5 < 10 hold, so && folds true; bound 3.
    // CHECK: and_true=6
    printf("and_true=%d\n", gateAnd<5>(three));

    // N = 12: 12 < 10 fails, so && folds false; bound 1.
    // CHECK: and_false=9
    printf("and_false=%d\n", gateAnd<12>(one));

    // N = 12: 12 > 10 holds, so || folds true; bound 1.
    // CHECK: or_true=9
    printf("or_true=%d\n", gateOr<12>(one));

    // N = 5: neither comparison holds, so || folds false; bound 3.
    // CHECK: or_false=6
    printf("or_false=%d\n", gateOr<5>(three));
}
