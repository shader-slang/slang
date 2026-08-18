//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#differentiation-autodiff
//META: doc_section_digest=cacccd6a57cbe2ad73776f1f2b2c77aba567defd4a6ab0580f1f60c7e9ad1561
//META: purpose=Reverse mode produces s_bwdProp_<original> for the propagation function and s_bwdCallableCtx_<original> for the intermediate-context struct that carries checkpointed primal values between the two halves.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both names the reverse-mode endpoint contributes to the emit,
// and the split between them. `sq` keeps an intermediate primal value in a
// local so the primal and propagation halves genuinely have something to
// hand across, which is what forces the context struct to exist. The
// CHECKs assert the propagation function and the context struct are each
// emitted under the documented name, and that the propagation function
// takes the context as a parameter — the link between the two halves.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

RWStructuredBuffer<float> outBuf;

[Differentiable]
float sq(float x)
{
    float y = x * x;
    return y * x;
}

[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    var dv = diffPair(float(tid.x), 0.0);
    bwd_diff(sq)(dv, 1.0);
    outBuf[0] = dv.d;
}

// The intermediate-context struct carrying checkpointed primal values ...
// HLSL-DAG: struct s_bwdCallableCtx_sq_{{[0-9]+}}
// ... and the propagation function that consumes it.
// HLSL-DAG: void s_bwdProp_sq_{{[0-9]+}}(s_bwdCallableCtx_sq_{{[0-9]+}}
