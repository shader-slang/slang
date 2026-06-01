//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E36100 (conflicting-capability-due-to-use-of-decl) - conflicting capability requirement
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36100
//META: catalog_name=conflicting-capability-due-to-use-of-decl
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A function declared with one target capability that calls a
// helper declared for a different incompatible target produces
// a conflict.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main -profile sm_6_0

[require(glsl)]
void onlyGlsl() {}

[require(hlsl)]
void caller()
{
    onlyGlsl();
}

[numthreads(1,1,1)]
void main()
{
    caller();
}

// CHECK: E36100
