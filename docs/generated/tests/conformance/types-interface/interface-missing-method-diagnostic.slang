//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=A struct that declares conformance to an interface but fails to provide a required method is rejected with E38100 (missing interface member).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a struct declaring conformance but omitting a required method
// produces E38100. Fails provides `something()` but not `mustProvide()`; CHECK
// pins the caret-aligned "missing interface member" message and E38100.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

interface IRequired
{
    int mustProvide();
}

struct Fails : IRequired
//CHECK:       ^^^^^^^^^ missing interface member
//CHECK:       ^^^^^^^^^ type 'Fails' does not provide required interface member 'mustProvide'
{
    int something() { return 0; }
}

void main() {}

// CHECK: E38100
