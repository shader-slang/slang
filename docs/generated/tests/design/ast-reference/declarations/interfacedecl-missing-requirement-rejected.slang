//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#requirementdecl-style-nodes-inside-interfacedecl
//META: doc_section_digest=61b41d7c34c3d57c8ab25ebc17ad4847ac05d606da5acd0ee3a97221870d03b4
//META: purpose=An interface requirement is whatever declaration was written inside the interface body; a conforming type that omits a required member is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that every declaration in an interface body is a requirement a
// conformer must satisfy. The struct declares a field but not the required
// method, so its conformance must fail; the CHECK pins the
// requirement-not-satisfied error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IShape { int area(); }

struct NotAShape : IShape
{
    int side;
//CHECK: error
//CHECK: E38100
}

void main() {}
