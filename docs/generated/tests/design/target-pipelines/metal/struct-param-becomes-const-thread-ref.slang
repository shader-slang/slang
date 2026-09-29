//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#undoparametercopy--transformparamstoconstref
//META: doc_section_digest=79ddec9513950e753aa3cb10c03d98ab4885a8fd448be4549c7698082e2ef35f
//META: purpose=`transformParamsToConstRef` converts a by-value struct parameter into a `const T thread*` pointer so Metal passes it by const reference.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `isCPUTarget || isCUDATarget || isMetalTarget` gate reaches
// Metal: a large read-only struct argument is turned into a const pointer
// rather than copied into the callee frame. The helper only reads its
// argument, and the caller mutates its own copy first so the two cannot be
// merged; CHECK asserts the emitted callee signature is
// `const Big_<N> thread*` and that its body dereferences through `->`.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

struct Big { float4 a; float4 b; float4 c; float4 d; };

uniform Big gBig;
RWStructuredBuffer<float> outBuf;

float sum(Big v) { return v.a.x + v.b.y + v.c.z + v.d.w; }

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Big local = gBig;
    local.a.x = float(tid.x);
    outBuf[tid.x] = sum(local);
}

// CHECK: float sum_{{[0-9]+}}(const Big_{{[0-9]+}} thread*
// CHECK: ->a_{{[0-9]+}}
