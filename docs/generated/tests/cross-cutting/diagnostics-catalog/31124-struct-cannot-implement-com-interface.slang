//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E31124 (struct-cannot-implement-com-interface) -- struct types cannot implement COM interfaces
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31124
//META: catalog_name=struct-cannot-implement-com-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[COM("11111111-1111-1111-1111-111111111111")]
interface IFoo {}
struct S : IFoo {}


// CHECK: E31124
