//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#requirementdecl-style-nodes-inside-interfacedecl
//META: doc_section_digest=4c37f9dfe6cf12e08ecfd91cb5b14070a8232c0e86d4bcb998f12ec60adc6aed
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
