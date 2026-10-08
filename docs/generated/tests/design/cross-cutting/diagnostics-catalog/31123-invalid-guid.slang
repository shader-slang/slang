//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=a8c63f994f633433f3521c7c3e2d13a6571e3455a05377a935b0362fe399079c
//META: purpose=Fires diagnostic E31123 (invalid-guid) -- invalid GUID
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31123
//META: catalog_name=invalid-guid
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[COM("not-a-guid")]
interface IFoo {}


//CHECK: E31123
