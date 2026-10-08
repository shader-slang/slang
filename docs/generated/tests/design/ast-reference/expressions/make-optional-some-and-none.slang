//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=f15f833400e70f33bd4f151e90be63f4887ec974cd69a44d2663e7a0d2ca3bdb
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
