//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=7254bc97bdcca63414dccb33e42076b021a7d40c0c49b389b0e55a42e9d38e5e
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
