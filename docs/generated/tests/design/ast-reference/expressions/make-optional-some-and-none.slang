//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=d2c639013cf026698511f9f1297a9f9de18bc449514e7113fc9483c45007d835
//META: purpose=A NoneLiteralExpr `none` yields the empty optional; an Optional<T> initialized from a T value yields the populated form via MakeOptionalExpr.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// NoneLiteralExpr is the `none` literal. Assigning `none` to an
// Optional<int> yields an empty optional whose `.hasValue` is
// false; assigning a wrapped value yields a populated optional.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<int> e = none;
    Optional<int> v = 42;
    // CHECK: e=0
    printf("e=%d\n", int(e.hasValue));
    // CHECK: v=42
    printf("v=%d\n", v.value);
}
