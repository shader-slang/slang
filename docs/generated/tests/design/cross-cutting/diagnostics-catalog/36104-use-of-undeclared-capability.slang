//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=f10111d688d4db27a4327f54fd4554f2a754e8e8d68be60a6a92dc615db870b2
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

//CHECK: E36104
