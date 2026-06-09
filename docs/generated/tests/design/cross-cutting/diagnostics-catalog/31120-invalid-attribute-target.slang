//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=044a4c343f92eaace41cb35d44d46513ec2b60f5c382a8644743605ca036c84f
//META: purpose=Fires diagnostic E31120 (invalid-attribute-target) -- invalid syntax target for user defined attribute
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31120
//META: catalog_name=invalid-attribute-target
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[__AttributeUsage(7)]
struct MyAttr {}


// CHECK: E31120
