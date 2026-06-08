//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#differentiation-autodiff
//META: doc_section_digest=f363d4438dd831bd4247c3b90982530b88aa7e9408107707c0706aef72c337c6
//META: purpose=The check-differentiability pass diagnoses calling a non-`[Differentiable]` function from a forward-derivative context.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Check differentiability" row of the differentiation table
// (`slang-ir-check-differentiability.cpp`) validates that flagged
// code is actually differentiable. Calling a non-`[Differentiable]`
// function from inside `fwd_diff(...)` triggers a warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<float> buf;

// Plain function, not marked differentiable.
float plain(float x) { return x * x; }

[Differentiable]
float wrap(float x) { return plain(x); }
//CHECK:                          ^ derivative cannot be propagated through non-differentiable call
// CHECK: E41022

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = wrap(float(tid.x));
}
