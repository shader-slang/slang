//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=585ce06912616ca8f54e5a07b259b7f8a9583cb64c6d8d3a4f63813ca94b9607
//META: purpose=Fires diagnostic E30810 (base-of-interface-must-be-interface) -- interface '~decl:Decl' cannot inherit from non-interface type '~baseType:Type'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30810
//META: catalog_name=base-of-interface-must-be-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S {}
interface I : S {}


//CHECK: E30810
