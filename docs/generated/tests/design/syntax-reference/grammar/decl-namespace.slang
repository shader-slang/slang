//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#top-level-structure
//META: doc_section_digest=341ea855b4cc01bfc612ef139eae7725ad7a366b921a54029383d73daf47f18f
//META: purpose=`NamespaceDecl` parses as `'namespace' IDENT '{' TopDecl* '}'` and qualifies names via `::`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the NamespaceDecl production and the `QualifiedName` form
// `IDENT '::' IDENT` used to refer to a name nested inside it.

//TEST:INTERPRET(filecheck=CHECK):

namespace ns
{
    static const int K = 11;
}

void main()
{
    // CHECK: K=11
    printf("K=%d\n", ns::K);
}
