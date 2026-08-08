//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#phase-d-cuda-emit-and-downstream-tools
//META: doc_section_digest=5fafdc04391fe1499bf74bfe01420955a8364571f1b55e362cc671d3444538c9
//META: purpose=A precise qualifier on a CUDA temporary is dropped with the PreciseQualifierUnsupportedOnTarget warning (E56005) because CUDA C++ has no precise keyword.
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the CUDA emitter diagnoses rather than silently honours
// `precise`. `emitTempModifiers` in the C++ emitter family (which the
// CUDA emitter inherits) finds the precise decoration on a temporary and
// reports it as unsupported. The CHECK pins the warning code E56005 on
// the declaration that carries the qualifier; the value itself still
// computes, so this is a dropped-qualifier warning and not an error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -entry computeMain -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    precise float t = float(tid.x) * 3.0f + 1.0f;
    //CHECK: E56005
    outBuf[tid.x] = t;
}
