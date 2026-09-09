//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b388676ebad1965b0d66e390b582ee73ceb7aeac4d5d6ebf5ae24b60d8325b20
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


//CHECK: E31124
