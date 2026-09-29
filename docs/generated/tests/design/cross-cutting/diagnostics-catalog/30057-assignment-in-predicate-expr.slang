//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=a110f7e97fd5267fe2b958e57d89a0a06a287da679a5c55ebf7adc2044f05516
//META: purpose=Fires diagnostic E30057 (assignment-in-predicate-expr) - assignment in predicate expression not allowed
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30057
//META: catalog_name=assignment-in-predicate-expr
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An assignment used where a condition is expected is nearly always a typo for
// `==`. Slang rejects it outright rather than converting the assigned value to
// bool, and the CHECK pins that error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { int x = 0; if (x = 1) {} }

//CHECK: E30057
