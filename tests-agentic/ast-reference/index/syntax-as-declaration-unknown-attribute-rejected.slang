//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:58:41Z
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/llm-generated/ast-reference/index.md#cross-cutting-topics
//META: doc_section_digest=36f1c960f3b34c6c84e64ac6cb1dc61adf1ed16c98013e2b8374b4a2928c2950
//META: purpose=The index doc names the syntax-as-declaration model (via SyntaxDecl and AttributeDecl) as the mapping from keywords and attributes to AST node classes; an attribute that has no declared mapping is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The #cross-cutting-topics section explicitly says that Slang's
// syntax-as-declaration model maps keywords and attributes to AST
// node classes (via SyntaxDecl and AttributeDecl). The
// cross-cutting consequence is that an attribute name that is not
// declared anywhere has no AST node class to map to, and the
// checker rejects it as an unknown attribute. This test cites the
// index's mapping-claim, not the per-attribute semantics page.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[ThisAttributeIsNotDeclaredAnywhere]
//CHECK: unknown attribute 'ThisAttributeIsNotDeclaredAnywhere'
// CHECK: W31000
void main()
{
}
