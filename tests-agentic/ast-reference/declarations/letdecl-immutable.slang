//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
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
}
