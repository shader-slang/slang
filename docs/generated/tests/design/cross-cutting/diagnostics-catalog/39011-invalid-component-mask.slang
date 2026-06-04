//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E39011 (invalid-component-mask) - invalid register component mask
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39011
//META: catalog_name=invalid-component-mask
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `packoffset(c0.q)` uses an invalid component letter and
// triggers E39011.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main -profile sm_5_0

cbuffer Cb
{
    float v : packoffset(c0.q);
}

[numthreads(1,1,1)]
void main() {}

// CHECK: E39011
