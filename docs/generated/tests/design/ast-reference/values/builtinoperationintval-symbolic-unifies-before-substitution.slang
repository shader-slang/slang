//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:00:25Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/values.md#builtinoperationintval-and-the-single-representation-invariant
//META: doc_section_digest=4229b55a01470c7ad99014966f3313e48dd3d9bca875bef59b0f7b4c86cbc182
//META: purpose=Two occurrences of the still-symbolic N / 2 name the same compile-time value inside the generic body; an array typed by one is accepted where the other is expected before any substitution happens.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the symbolic form of a builtin operation is a single
// canonical value: forward's parameter int[N / 2] is passed to
// helperSum<N>, whose own parameter is int[M / 2] with M bound to the
// same N. The call inside the un-instantiated generic body checks only
// if the two symbolic division values unify; CHECK asserts the value
// still flows through a concrete instantiation.

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
