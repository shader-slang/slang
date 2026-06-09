//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E41201 (expect-dynamic-uniform-argument) - argument might not be dynamic uniform
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41201
//META: catalog_name=expect-dynamic-uniform-argument
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Passing a per-thread (non-uniform) value to a `dynamic_uniform`
// parameter triggers the uniformity-validation warning under
// -validate-uniformity.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -profile cs_6_0 -validate-uniformity

RWStructuredBuffer<int> output;

void expectUniform(dynamic_uniform int v) { output[0] = v; }

void main(int tid : SV_DispatchThreadID) {
    expectUniform(tid);
}

// CHECK: E41201
