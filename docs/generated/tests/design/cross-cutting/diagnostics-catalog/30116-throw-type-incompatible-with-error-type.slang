//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=21d8c4b03035b95ae0dfc839fc64e1ac175cc5a7d9723293435fe5cc5f5d698a
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

//CHECK: 30116
