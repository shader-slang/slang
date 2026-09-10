//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#container-level-cap
//META: doc_section_digest=17336538ec29e57920f3b7003dd39a6ad2e2af3018c615ad8a7112f747ce1b92
//META: purpose=A `public` function whose return type is less-visible (an `internal` struct) is rejected with `use-of-less-visible-type`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Container-level cap: a decl cannot reference a type that is less
// visible than itself. `IS` is `internal` (modern default); a public
// function returning it fires diagnostic 30604.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct IS {}

public IS makeIS() {}
//CHECK:  ^^^^^^ references less visible type
//CHECK:  ^^^^^^ 'makeIS' references less visible type 'IS'.
// CHECK: E30604
