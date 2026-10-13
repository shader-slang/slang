//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=f012520419b56e029d328a83f4eebf64ed2d1f66cc3c1f189c259597597ba07e
//META: purpose=Fires diagnostic E30060 (expected-a-type) - expected a type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30060
//META: catalog_name=expected-a-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A declaration's first token must name a type. Using a value-bound identifier
// there makes the checker report that a type was expected, which the CHECK
// pins by code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { int x = 0; x y; }

//CHECK: E30060
