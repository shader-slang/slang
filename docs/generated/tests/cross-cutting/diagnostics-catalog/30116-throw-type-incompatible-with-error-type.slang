//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30116 (throw-type-incompatible-with-error-type) - throw type incompatible with error type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30116
//META: catalog_name=throw-type-incompatible-with-error-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Throwing a value whose type doesn't match the declared error type triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct MyErr {}

int f() throws MyErr
{
    throw 1;
}

void main() {}

// CHECK: 30116
