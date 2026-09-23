//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=201f43c1d7c5cfcc77b1b4ce6fc2cf8382c914afa0e42880e9913a9b956fe0a6
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

//CHECK: 30113
