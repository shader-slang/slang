//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=232bb23f0daa18d59b047831901c0b444bf01283e5c09bd1d859fc8824f19b7c
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
