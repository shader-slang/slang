//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=04db9a2923778ace79480c7cf26e7e55d495823802e46a17b3603df329381f62
//META: purpose=Fires diagnostic E31125 (attribute-usage-attribute-must-be-on-non-generic-struct) -- [__AttributeUsage] requires non-generic struct
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31125
//META: catalog_name=attribute-usage-attribute-must-be-on-non-generic-struct
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[__AttributeUsage(1)]
struct MyAttr<T> {}


//CHECK: E31125
