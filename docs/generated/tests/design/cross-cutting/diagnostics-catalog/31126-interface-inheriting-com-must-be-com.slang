//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=253fe9d71f6b0888883d9250db9e8a1ca98d0282bd4dc99a1a23b08f29a90711
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


//CHECK: E31126
