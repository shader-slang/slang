//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30814 (base-of-class-must-be-class-or-interface) -- class '~decl:Decl' cannot inherit from type '~baseType:Type' that is neither a class nor an interface
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30814
//META: catalog_name=base-of-class-must-be-class-or-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

class C : int {}


// CHECK: E30814
