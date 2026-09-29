//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=b49700d31492527c686bc2846076b0dd0110de840c0102148f8bbf876b2bcdb4
//META: purpose=Boundary: an `enum` with `int`-typed underlying values accepts the documented int MIN (-2147483648) and int MAX (2147483647) as explicit case values.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stresses the explicit-value form of `enum` at the boundary of the
// underlying `int` type. The parser must accept both INT_MIN and
// INT_MAX as compile-time case initializers.

//TEST:INTERPRET(filecheck=CHECK):

enum Bounds : int
{
    Lo = -2147483648,
    Hi = 2147483647,
};

void main()
{
    // CHECK: lo=-2147483648 hi=2147483647
    printf("lo=%d hi=%d\n", int(Bounds.Lo), int(Bounds.Hi));
}
