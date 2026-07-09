//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=4c63870a8c35012e1b74f71eacefdafafbaf651311a8881d0748fe0533759d6e
//META: purpose=Fires diagnostic E30813 (base-of-extension-must-be-interface) -- extension cannot inherit from non-interface type '~baseType:Type'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30813
//META: catalog_name=base-of-extension-must-be-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S {}
struct T {}
extension S : T {}


// CHECK: E30813
