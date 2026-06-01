//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E36104 (use-of-undeclared-capability) - undeclared capability used
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36104
//META: catalog_name=use-of-undeclared-capability
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A function declared with [require(hlsl)] that calls a builtin
// requiring sm_6_0 without declaring it triggers E36104.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target glsl -entry main -stage compute -profile sm_5_0

[require(hlsl)]
void caller()
{
    AllMemoryBarrier();
}

[numthreads(1,1,1)]
void main()
{
    caller();
}

// CHECK: E36104
