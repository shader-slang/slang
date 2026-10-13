//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:55:00Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#breadcrumbs
//META: doc_section_digest=e714c878daa0803c32b68e81de8e900c7c57b11fdf6b046adec786e715edc76c
//META: purpose=_lookUpInScopes records ThisValue for an unqualified field in an instance method; ConstructLookupResultExpr replays it, and checkThisExpr determines the implicit receiver's type and value category.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `_lookUpInScopes` records one `ThisValue` step for `g`. `ConstructLookupResultExpr` replays that
// step and calls `checkThisExpr`, which determines the implicit receiver's type and value category
// in the lookup-origin scope before the checker forms the member access. The result confirms that
// the unqualified read uses the caller's object rather than an unrelated default-initialized value.

//TEST:INTERPRET(filecheck=CHECK):

struct S
{
    int g;
    int read() { return g; }
}

void main()
{
    S s;
    s.g = 33;
    // CHECK: r=33
    printf("r=%d\n", s.read());
}
