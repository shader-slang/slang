//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E36117 (decl-has-dependencies-not-compatible-on-stage) - dependencies not compatible on stage
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36117
//META: catalog_name=decl-has-dependencies-not-compatible-on-stage
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A compute entry point calling a fragment-only intrinsic
// triggers E36117 (stage missing).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -entry main -stage compute -profile sm_6_0

[require(hlsl, fragment)]
void onlyFrag()
{
}

[numthreads(1,1,1)]
void main()
{
    onlyFrag();
}

// CHECK: E36117
