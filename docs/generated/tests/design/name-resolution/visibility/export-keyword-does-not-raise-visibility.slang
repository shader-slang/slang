//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#interaction-with-extern-and-export
//META: doc_section_digest=403fe2795d9f861f86a9b27c1535faca2b2c8fd96e1ba9fff4f9858b4f7b550f
//META: purpose=The bare `export` keyword records linkage intent and does not raise a decl's visibility, so an `export`-marked struct is still only `internal` to the visibility checker.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `export` and the `public`/`internal`/`private` family are
// independent axes. `IS` is marked `export` but carries no visibility
// modifier, so it keeps the module default `internal`. If `export` promoted
// it to `public`, the `public` function naming `IS` would check cleanly; the
// CHECK pins the E30604 less-visible-type rejection that proves the
// classification was unchanged.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

export struct IS { int v; }

public IS makeIS(IS s) { return s; }
//CHECK:  ^^^^^^ references less visible type
//CHECK:  ^^^^^^ 'makeIS' references less visible type 'IS'.
// CHECK: E30604
