//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=A struct that fails to provide a required associated type is rejected with E38100 (missing interface member) naming the missing associated type.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that omitting a required associated type from a conforming struct
// produces E38100. FailsAssoc declares no ElemType; CHECK pins the caret-aligned
// "missing interface member" message and E38100 on the struct declaration line.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

interface IHasAssoc
{
    associatedtype ElemType;
}

struct FailsAssoc : IHasAssoc
//CHECK:            ^^^^^^^^^ missing interface member
//CHECK:            ^^^^^^^^^ type 'FailsAssoc' does not provide required interface member 'ElemType'
{
}

void main() {}
