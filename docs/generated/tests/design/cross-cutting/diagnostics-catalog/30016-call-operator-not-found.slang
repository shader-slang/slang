//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=745c7ff2bcd3a21cba48cf38ba415e6bbeca1d3ec3d7dd7b8b8eeaf8c6d4c094
//META: purpose=Fires diagnostic E30016 (call-operator-not-found) - no call operation found for type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30016
//META: catalog_name=call-operator-not-found
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A call expression requires a callable callee. An `int` local has no call
// operator, so the CHECK pins the no-call-operation error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { int x = 0; x(); }

//CHECK: E30016
