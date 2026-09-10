//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=a28bf8882ad3b2018607836f5ceb2505c88613301a9aa06f6c1cec80eee5ac3f
//META: purpose=Fires diagnostic E30095 (error-type-of-callee-incompatible-with-caller) - incompatible error types
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30095
//META: catalog_name=error-type-of-callee-incompatible-with-caller
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Callee's error type doesn't match caller's declared error type.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct ErrA {}
struct ErrB {}

int inner() throws ErrA { throw ErrA(); }

int outer() throws ErrB
{
    int v = try inner();
    return v;
}

void main() {}

//CHECK: 30095
