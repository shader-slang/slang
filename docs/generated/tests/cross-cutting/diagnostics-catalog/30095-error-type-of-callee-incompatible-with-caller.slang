//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
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

// CHECK: 30095
