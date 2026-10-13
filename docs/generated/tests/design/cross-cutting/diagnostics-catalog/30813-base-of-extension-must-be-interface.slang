//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=7a9c45c780996798d2ce8251740575ad498b4995355b5a5dcb404d7683b75f84
//META: purpose=Fires diagnostic E30813 (base-of-extension-must-be-interface) -- extension cannot inherit from non-interface type '~baseType:Type'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30813
//META: catalog_name=base-of-extension-must-be-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S {}
struct T {}
extension S : T {}


//CHECK: E30813
