//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=f5f69758467034452c4b9f6f55930173c5638f1de6b2d3ccd86bcbbfd9021ada
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

//CHECK: 3009
