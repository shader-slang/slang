//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
//META: purpose=A LetDecl is a let variable; immutable. Assignment through a let-bound name is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of LetDecl ("immutable"). Trying to
// assign through a let-bound local must produce a diagnostic; the
// documented immutability of `let` is the user-observable consequence
// of the LetDecl class.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    let x = 1;
    x = 2;
/*CHECK:
      ^ left of '=' is not an l-value
*/
// CHECK: E30011
}
