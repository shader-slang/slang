//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#builtinoperationintval-and-the-single-representation-invariant
//META: doc_section_digest=7294e983132681667c88eb7937aa63061a1f9c975bb9567ce8dd36184dcda775
//META: purpose=The short-circuit operators are resolved-operator-call forms of the symbolic builtin-operation value, so && and || over a generic value parameter fold with the enclosing ?: to a concrete array bound.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `&&` and `||` evaluate at compile time once their
// generic operands are substituted. gateAnd's bound selects 3 only when
// both comparisons hold; gateOr's selects 1 only when either holds.
// Each call type-checks against the folded bound, and CHECK asserts the
// sums from both branches of both operators.

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
