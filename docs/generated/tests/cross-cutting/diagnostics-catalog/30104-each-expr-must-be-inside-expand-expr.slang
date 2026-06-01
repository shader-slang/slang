//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30104 (each-expr-must-be-inside-expand-expr) - 'each' must be inside 'expand'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30104
//META: catalog_name=each-expr-must-be-inside-expand-expr
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `each` outside an `expand` expression is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void f<each T>()
{
    int x = each T;
}

void main() {}

// CHECK: 30104
