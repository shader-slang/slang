//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=ebb51fe21184f583b3099af3f7a104b6772917a9532bb84d08e9ed26a0e93ac5
//META: purpose=Fires diagnostic E31201 (modifier-not-allowed) -- modifier not allowed
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31201
//META: catalog_name=modifier-not-allowed
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main() {
    nointerpolation int x = 5;
}


//CHECK: E31201
