//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30106 (improper-use-of-type) - type cannot be used in this context
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30106
//META: catalog_name=improper-use-of-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using a type name in a value-expression context is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S {};

void main()
{
    int x = S + 1;
}

// CHECK: 30106
