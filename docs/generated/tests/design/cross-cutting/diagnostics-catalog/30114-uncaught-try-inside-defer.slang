//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=8516e40e8ccea03d030fd26f655eaec94a224310d0d91bf8cc096600127d9b62
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

//CHECK: 30114
