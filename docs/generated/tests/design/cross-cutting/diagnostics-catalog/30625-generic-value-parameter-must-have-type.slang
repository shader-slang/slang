//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=c12bc2a5b9ac01436778d9b7dc69ceec7279121d7bddd2d4a7e7ebc7939130da
//META: purpose=Fires diagnostic E30625 (generic-value-parameter-must-have-type) -- a generic value parameter must be given an explicit type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30625
//META: catalog_name=generic-value-parameter-must-have-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S<let x> {}


//CHECK: E30625
