//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Drop placeholder for E30068 (mutating-method-on-function-input-parameter-warning); see README.md
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30068
//META: catalog_name=mutating-method-on-function-input-parameter-warning
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// E30068 is the warning variant of E30067 covered by another file; the warning
// path could not be isolated. Asserts that a trivial program does not spuriously
// emit E30068. Documented in README.md "Codes dropped".

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main() {}

// CHECK-NOT: E30068
