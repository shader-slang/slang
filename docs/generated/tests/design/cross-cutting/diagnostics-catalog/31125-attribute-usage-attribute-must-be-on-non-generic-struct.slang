//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=002850a1e7798bb9a229794c172883820d064f025abda27bce6d759ef864563d
//META: purpose=Fires diagnostic E31125 (attribute-usage-attribute-must-be-on-non-generic-struct) -- [__AttributeUsage] requires non-generic struct
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31125
//META: catalog_name=attribute-usage-attribute-must-be-on-non-generic-struct
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[__AttributeUsage(1)]
struct MyAttr<T> {}


// CHECK: E31125
