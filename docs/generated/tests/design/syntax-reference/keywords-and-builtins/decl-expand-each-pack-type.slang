//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T00:00:00Z
//META: source_commit=b15b8060d2525b8cd123f2268fafbcd82e837fb3
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=e3d06ef8bf7a4508b0ecb0515b3867979078821ac9b8143e7b039cd76e1b73b1
//META: purpose=The type-specifier parser recognizes the variadic-pack type forms `expand` and `each` by direct identifier lookahead; an `expand each T` parameter accepts an argument pack whose element count `countof(T)` reflects the call site.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `expand` and `each` act as variadic-pack type forms in a
// parameter's type specifier. A generic with an `each T` pack and an
// `expand each T` parameter is called with three arguments of three
// distinct types; CHECK asserts countof(T) observed inside the callee
// is 3, which requires both identifiers to have parsed as the pack
// type forms rather than ordinary names.

//TEST:INTERPRET(filecheck=CHECK):

int packLen<each T>(expand each T args)
{
    return countof(T);
}

void main()
{
    // CHECK: n=3
    printf("n=%d\n", packLen(1, 2.0f, true));
}
