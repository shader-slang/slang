//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:58:54+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ast-reference/values.md#builtinoperationintval-and-the-single-representation-invariant
//META: doc_section_digest=7294e983132681667c88eb7937aa63061a1f9c975bb9567ce8dd36184dcda775
//META: purpose=Operators on enum operands are the other resolved-operator-call form that reaches the symbolic builtin-operation value, so an enum comparison over a generic value parameter folds to a concrete array bound.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the operand type does not change the outcome: the
// comparison here is between two enum-typed values (the enum-typed
// generic value parameter N and Size.Large), not between plain ints.
// Two instantiations select different branches of the enclosing ?:, and each call
// type-checks only against the branch its condition folds to; CHECK
// asserts both runtime sums.

//TEST:INTERPRET(filecheck=CHECK):

enum Size { Small = 2, Large = 6 }

int pick<let N : Size>(int a[(N == Size.Large) ? 3 : 1])
{
    int total = 0;
    for (int i = 0; i < ((N == Size.Large) ? 3 : 1); ++i) { total += a[i]; }
    return total;
}

void main()
{
    int three[3] = { 1, 2, 3 };
    int one[1] = { 9 };

    // N = Size.Large compares equal to Size.Large, so it folds true and the bound is 3.
    // CHECK: large=6
    printf("large=%d\n", pick<Size.Large>(three));

    // N = Size.Small compares unequal to Size.Large, so it folds false and the bound is 1.
    // CHECK: small=9
    printf("small=%d\n", pick<Size.Small>(one));
}
