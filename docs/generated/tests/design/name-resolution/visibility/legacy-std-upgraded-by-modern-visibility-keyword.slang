//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#concepts
//META: doc_section_digest=486dea2f46d1d57027a213610a753c17e6b52cde71a165cabb52ac946a910a45
//META: purpose=A file requested as legacy (`-std 2018`) is upgraded away from the legacy language version when it uses a modern construct, so unannotated decls default to `internal` rather than the legacy `public`.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the resolved-language-version step: the compile request asks for
// the legacy version, whose documented default makes every unannotated decl
// `public`. If that default survived, `IS` would be public and the `public`
// function naming it would check cleanly. The CHECK pins the E30604
// less-visible-type rejection instead, which is only reachable if the module
// resolved to a modern language version (default `internal`) because the file
// uses the `public` keyword.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 2018

struct IS { int v; }

public IS makeIS(IS s) { return s; }
//CHECK:  ^^^^^^ references less visible type
//CHECK:  ^^^^^^ 'makeIS' references less visible type 'IS'.
// CHECK: E30604
