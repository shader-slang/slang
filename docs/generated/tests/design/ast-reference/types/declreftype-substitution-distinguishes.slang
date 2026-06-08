//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#declreftype
//META: doc_section_digest=354aa5e4ad824810f192137554fb86892215b9326ffe9fad3435b95088dbb52d
//META: purpose=Two DeclRefTypes that reference the same generic StructDecl with different generic substitutions are distinct types; one cannot be assigned to the other.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `DeclRefType` note states: "carries a decl-ref to a
// type declaration ... together with whatever generic substitutions
// apply". Two substitutions of the same generic decl are distinct
// `DeclRefType` instances and the checker enforces the distinction:
// a `Pair<int>` cannot be assigned to a `Pair<float>`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Pair<T> { T first; T second; }

void main()
{
    Pair<int> pi;
    Pair<float> pf = pi;
//CHECK:             ^^ expected an expression of type 'Pair<float>', got 'Pair<int>'
//CHECK: E30019
}
