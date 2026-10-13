//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=d5783d2a624119a0f15cb857c4f4b4401b0afa13461390dd9d8a8f32af79752a
//META: purpose=Fires diagnostic E30811 (base-of-struct-must-be-interface) -- struct '~decl:Decl' cannot inherit from non-interface type '~baseType:Type'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30811
//META: catalog_name=base-of-struct-must-be-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

enum E { A }
struct S : E {}


//CHECK: E30811
