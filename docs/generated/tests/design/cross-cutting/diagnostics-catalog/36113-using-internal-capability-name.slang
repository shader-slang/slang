//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=37742a724e165a46226aa980b9cbde8042d7ea0180f921a00f6db0bbfc6dc7f6
//META: purpose=Fires diagnostic E36113 (using-internal-capability-name) - using internal capability name
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36113
//META: catalog_name=using-internal-capability-name
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using the internal capability name `_sm_4_0` (which has a
// leading underscore) triggers E36113. Public users are expected
// to use `sm_4_0` instead.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target glsl -entry main -stage compute -profile sm_5_0

[require(glsl, _sm_4_0)]
public void helper() {}

[numthreads(1,1,1)]
void main() { helper(); }

//CHECK: E36113