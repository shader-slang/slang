//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic W36113 (using-internal-capability-name) - using internal capability name
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36113
//META: catalog_name=using-internal-capability-name
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using the internal capability name `_sm_4_0` (which has a
// leading underscore) triggers W36113. Public users are expected
// to use `sm_4_0` instead.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target glsl -entry main -stage compute -profile sm_5_0

[require(glsl, _sm_4_0)]
public void helper() {}

[numthreads(1,1,1)]
void main() { helper(); }

// CHECK: W36113
