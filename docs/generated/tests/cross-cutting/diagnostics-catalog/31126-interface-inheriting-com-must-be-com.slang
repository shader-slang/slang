//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E31126 (interface-inheriting-com-must-be-com) -- non-COM interface inheriting from COM interface
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31126
//META: catalog_name=interface-inheriting-com-must-be-com
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[COM("11111111-1111-1111-1111-111111111111")]
interface IFoo {}
interface IBar : IFoo {}


// CHECK: E31126
