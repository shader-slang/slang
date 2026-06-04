//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=5e08a46be959aef3bcc869c23de58efd691cf6ceb1efa6cff0d9265f8cea893d
//META: purpose=Fires diagnostic E30810 (base-of-interface-must-be-interface) -- interface '~decl:Decl' cannot inherit from non-interface type '~baseType:Type'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30810
//META: catalog_name=base-of-interface-must-be-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S {}
interface I : S {}


// CHECK: E30810
