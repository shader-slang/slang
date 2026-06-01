//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E36116 (capability-has-multiple-stages) - capability targets multiple stages
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36116
//META: catalog_name=capability-has-multiple-stages
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using a `[shader("...")]` value that resolves to a multi-stage
// capability triggers E36116.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -entry main -allow-glsl -profile sm_5_0

[shader("compute_fragment")]
void main()
{
}

// CHECK: E36116
