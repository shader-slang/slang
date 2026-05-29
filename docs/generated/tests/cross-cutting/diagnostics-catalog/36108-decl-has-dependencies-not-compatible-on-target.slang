//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E36108 (decl-has-dependencies-not-compatible-on-target) - dependencies not compatible on target
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36108
//META: catalog_name=decl-has-dependencies-not-compatible-on-target
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Compiling for HLSL but declaring [require(glsl)] dependencies
// triggers E36108.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -entry main -stage compute -profile sm_6_0

[require(glsl)]
public void use1()
{
}

[numthreads(1,1,1)]
void main()
{
    use1();
}

// CHECK: E36108
