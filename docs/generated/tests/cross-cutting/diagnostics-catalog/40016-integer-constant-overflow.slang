//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: E40016
