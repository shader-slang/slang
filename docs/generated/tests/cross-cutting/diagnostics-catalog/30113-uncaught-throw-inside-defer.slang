//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30113 (uncaught-throw-inside-defer) - 'throw' requires 'catch' inside defer
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30113
//META: catalog_name=uncaught-throw-inside-defer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `throw` inside `defer` requires a matching local catch.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void f() throws int
{
    defer { throw 1; };
}

void main() {}

// CHECK: 30113
