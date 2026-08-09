//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#builtinoperationintval-and-the-single-representation-invariant
//META: doc_section_digest=7294e983132681667c88eb7937aa63061a1f9c975bb9567ce8dd36184dcda775
//META: purpose=One logical compile-time value has exactly one symbolic spelling, so two occurrences of the still-unsubstituted N / 2 name the same value and the array types they bound are interchangeable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the single-representation invariant on the symbolic side:
// forward's parameter int[N / 2] is passed to helperSum<N>, whose own
// parameter is int[M / 2] with M bound to the same N. That call is
// checked inside the un-instantiated generic body, so it succeeds only
// if the two symbolic divisions are the same value; CHECK asserts the
// value still flows through a concrete instantiation.

//TEST:INTERPRET(filecheck=CHECK):

int helperSum<let M : int>(int b[M / 2])
{
    int total = 0;
    for (int i = 0; i < M / 2; ++i) { total += b[i]; }
    return total;
}

int forward<let N : int>(int a[N / 2])
{
    // Checked before substitution: int[N / 2] must be the same type
    // as helperSum<N>'s int[N / 2].
    return helperSum<N>(a);
}

void main()
{
    int aa[4] = { 1, 2, 3, 4 };
    // CHECK: fwd=10
    printf("fwd=%d\n", forward<8>(aa));
}
