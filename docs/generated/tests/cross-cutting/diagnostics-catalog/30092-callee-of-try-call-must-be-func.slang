//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30092 (callee-of-try-call-must-be-func) - callee in 'try' must be a function
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30092
//META: catalog_name=callee-of-try-call-must-be-func
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `try` applied to a non-call expression fires the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int wrapper() throws int
{
    int x = try 42;
    return x;
}

void main() {}

// CHECK: 3009
