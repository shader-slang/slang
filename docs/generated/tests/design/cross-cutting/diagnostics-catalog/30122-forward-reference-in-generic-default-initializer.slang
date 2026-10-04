//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2280833a7ec2662cc99b1fe5b8b0863a0d1eca139b4a8eed7545237cc48b2702
//META: purpose=Fires diagnostic E30122 (forward-reference-in-generic-default-initializer) - forward reference in generic default initializer
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30122
//META: catalog_name=forward-reference-in-generic-default-initializer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A generic value parameter default initializer that references a later parameter is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void f<let A : int = B, let B : int>() {}

void main() {}

//CHECK: 30122
