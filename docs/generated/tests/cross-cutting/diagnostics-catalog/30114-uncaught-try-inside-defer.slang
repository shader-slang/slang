//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30114 (uncaught-try-inside-defer) - 'try' requires 'catch' inside defer
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30114
//META: catalog_name=uncaught-try-inside-defer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `try` inside `defer` requires a matching local catch.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int may_fail() throws int { throw 1; }

void f() throws int
{
    defer { int v = try may_fail(); };
}

void main() {}

// CHECK: 30114
