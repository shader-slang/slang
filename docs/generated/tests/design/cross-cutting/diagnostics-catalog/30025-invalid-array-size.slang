//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=de3b36c2a65387e3f2b6f88aa4d905eb9489a6ffc5c62a9c309818248b13e6b6
//META: purpose=Fires diagnostic E30025 (invalid-array-size) - array size must be non-negative
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30025
//META: catalog_name=invalid-array-size
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An array's element count must be non-negative. A literal `-1` extent is
// folded at check time and rejected, which the CHECK pins by code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { int a[-1]; }

//CHECK: E30025
