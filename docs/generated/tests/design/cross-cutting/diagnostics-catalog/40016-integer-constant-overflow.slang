//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=95ec324ebebd51eeebd7f5aa09e00db21209eff4ad2b3a33e5ffc6aa9b262d52
//META: purpose=Fires diagnostic E40016 (integer-constant-overflow) - integer constant overflow in conversion
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=40016
//META: catalog_name=integer-constant-overflow
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Assigning the literal 300 to a uint8_t triggers the integer-constant
// overflow warning during conversion.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

[numthreads(1,1,1)]
void main() {
    uint8_t x = 300;
}

//CHECK: E40016
