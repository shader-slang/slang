//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
//META: purpose=A ConstructorDecl's signature is part of the type's API; calling the constructor with the wrong arity is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative form of the ConstructorDecl role: the
// constructor's parameter list is part of the type's API, so a call
// with the wrong number of arguments cannot resolve and the checker
// must diagnose it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct Wrap
{
    int v;
    __init(int x) { v = x; }
}

void main()
{
    Wrap w = Wrap(1, 2);
//CHECK: error
//CHECK: E39999
}
