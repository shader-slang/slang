//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:13Z
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ast-reference/values.md#declref-family
//META: doc_section_digest=20c2eb71a7785d9324495ca71a49afdc3bcf69ae059047eeb0dd7dadf9b245cc
//META: purpose=Two GenericAppDeclRefs of the same decl with different generic arguments are distinct types; assigning one to the other is rejected by the type checker.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc: `GenericAppDeclRef` "wraps an existing decl-ref in
// generic-argument substitutions". Distinct argument lists
// produce distinct decl-refs and therefore distinct types; the
// checker rejects cross-instantiation assignment.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Box<T> { T v; }

void main()
{
    Box<int> bi;
    Box<float> bf = bi;
//CHECK:            ^^ expected an expression of type 'Box<float>', got 'Box<int>'
//CHECK: E30019
}
