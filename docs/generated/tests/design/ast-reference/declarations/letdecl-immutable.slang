//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
//META: purpose=A LetDecl is a `let` variable and is immutable; assignment through a let-bound name is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `let` binding is immutable. The test assigns through a
// let-bound local; the CHECK pins the l-value diagnostic (E30011) at the
// name, which is the only user-visible way immutability manifests.

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
