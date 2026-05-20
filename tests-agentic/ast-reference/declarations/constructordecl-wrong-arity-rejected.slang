//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
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
}
