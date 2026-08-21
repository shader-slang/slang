//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=ae6e224e9a59d941ed9501cea9cc7ff193438c55083d6dfcd4c64e040681a055
//META: purpose=Fires diagnostic E30003 (break-outside-loop) - 'break' must appear inside loop or switch constructs
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30003
//META: catalog_name=break-outside-loop
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `break` needs an enclosing loop or `switch` to exit. At function-body scope
// there is none, and the CHECK pins the break-outside-loop error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { break; }

//CHECK: E30003
