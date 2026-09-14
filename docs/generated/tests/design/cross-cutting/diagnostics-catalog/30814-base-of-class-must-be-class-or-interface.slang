//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=8224cac718a183c057bd1ab0f40f5893d9e8951eb310b98819ab08214133bcff
//META: purpose=Fires diagnostic E30814 (base-of-class-must-be-class-or-interface) -- class '~decl:Decl' cannot inherit from type '~baseType:Type' that is neither a class nor an interface
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30814
//META: catalog_name=base-of-class-must-be-class-or-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

class C : int {}


//CHECK: E30814
