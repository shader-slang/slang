//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#requirementdecl-style-nodes-inside-interfacedecl
//META: doc_section_digest=d2ddd242ca9bb660cb1ae0f8e2b2e49371d56501836a7e6ac082e8f7f550fbfd
//META: purpose=An interface requirement is whatever Decl is written inside the interface body; a conforming type that omits a required member is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented behavior that "an interface requirement is
// whatever Decl was written inside the interface body". The struct
// fails to define `area`, so its `: IShape` conformance must fail and
// the checker must diagnose it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IShape { int area(); }

struct NotAShape : IShape
{
    int side;
//CHECK: error
//CHECK: E38100
}

void main() {}
