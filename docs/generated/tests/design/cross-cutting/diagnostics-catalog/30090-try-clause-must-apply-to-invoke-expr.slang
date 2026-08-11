//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2b66df4ed1fd7b0097862b50a2d0f1d03da98f3a93ce94c28d7a3a8e78ab0918
//META: purpose=Fires diagnostic E30090 (try-clause-must-apply-to-invoke-expr) - expression in 'try' must be a call
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30090
//META: catalog_name=try-clause-must-apply-to-invoke-expr
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `try` propagates an error out of a call, so its operand must be a call
// expression. Applying it to a plain variable reference is rejected, which the
// CHECK observes by code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { int x = 0; try x; }

//CHECK: E30090
