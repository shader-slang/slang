//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2d9c0e8de3d8237fe99a57ac8f59daef3a6a686010401e36388ac766785e66d1
//META: purpose=Fires diagnostic E15900 (user-defined-error) - preprocessor error
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15900
//META: catalog_name=user-defined-error
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `#error` turns its message into a compiler error at the point it is reached.
// Placing it unconditionally at file scope fires it, and the CHECK pins the
// user-defined-error code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#error deliberate user-defined preprocessor error
void main() {}

//CHECK: E15900
