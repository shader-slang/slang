//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30020 (expected-array-expression) - expected an array expression
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30020
//META: catalog_name=expected-array-expression
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Indexing a scalar with [] triggers the array-expression diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int x = 5;
    int y = x[0];
}

// CHECK: E30020
