//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b26c53a58827a63a4737a5ffecaebc522ed5c26fb08cf18d16f61f7f73fc1386
//META: purpose=Fires diagnostic E31123 (invalid-guid) -- invalid GUID
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31123
//META: catalog_name=invalid-guid
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[COM("not-a-guid")]
interface IFoo {}


// CHECK: E31123
