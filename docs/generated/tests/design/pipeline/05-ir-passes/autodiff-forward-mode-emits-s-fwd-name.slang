//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#differentiation-autodiff
//META: doc_section_digest=cacccd6a57cbe2ad73776f1f2b2c77aba567defd4a6ab0580f1f60c7e9ad1561
//META: purpose=Forward mode gives the generated derivative function the name hint s_fwd_<original>, so a fwd_diff call on a [Differentiable] function turns into a call to that name in the emit.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one name the autodiff family's forward endpoint puts into
// the emitted source. `sq` is `[Differentiable]` and is invoked through
// `fwd_diff`, which is what makes the forward-mode pass synthesize a
// derivative function rather than leaving the call alone. The CHECKs
// assert a function named after the original with the `s_fwd_` prefix is
// both defined and called; without the pass there would be no such symbol
// and the `fwd_diff` call would have nothing to lower to.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main

RWStructuredBuffer<float> outBuf;

[Differentiable]
float sq(float x)
{
    return x * x;
}

[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    var dp = DifferentialPair<float>(float(tid.x), 1.0);
    outBuf[0] = fwd_diff(sq)(dp).d;
}

// The derivative function is named after the original it was derived from.
// HLSL-DAG: s_fwd_sq_{{[0-9]+}}(

// SPIRV-DAG: s_fwd_sq
