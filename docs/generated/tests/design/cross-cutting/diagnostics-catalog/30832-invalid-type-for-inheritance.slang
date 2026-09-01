//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=5d6ee6445b71c93c1ebf8a8fb3ba37b51c110bd16bcc3fc9f486dc04b16e47f9
//META: purpose=Fires diagnostic E30832 (invalid-type-for-inheritance) -- type '~type:Type' cannot be used for inheritance
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30832
//META: catalog_name=invalid-type-for-inheritance
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S<T> : T {}


//CHECK: E30832
