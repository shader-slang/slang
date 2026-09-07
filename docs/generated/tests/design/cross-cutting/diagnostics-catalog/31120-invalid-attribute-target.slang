//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=031139ce9a8a50f6d5c0b00dbae7f6ad82b5d43b5d56a3bb7dfc548b9d072abd
//META: purpose=Fires diagnostic E31120 (invalid-attribute-target) -- invalid syntax target for user defined attribute
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31120
//META: catalog_name=invalid-attribute-target
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[__AttributeUsage(7)]
struct MyAttr {}


//CHECK: E31120
