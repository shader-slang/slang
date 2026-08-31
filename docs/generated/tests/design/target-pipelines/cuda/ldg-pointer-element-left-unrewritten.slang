//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda
//META: doc_section_digest=18d5561ebccbbde662de4ad1843df9fd129444c9391f63438c09ff9b8432a453
//META: purpose=Boundary - a load whose leaf is an opaque type such as a pointer falls off the end of createLoadFuncForType's switch, so processInst leaves the original StructuredBuffer load exactly as it found it and no __ldg is emitted.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the unrewritten arm that makes this pass a best-effort
// optimization rather than a legalization. A pointer element type
// matches none of the five recognized type ops, so the rewrite
// yields an empty load method and the load survives untouched. The
// CHECK asserts the buffer read still emits as an ordinary .Load
// call and that no __ldg appears anywhere in the artifact — if the
// pass had rewritten it, correctness would be unchanged but the
// read-only-cache intrinsic would be there.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

StructuredBuffer<float*> roBuf;
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float* p = roBuf[tid.x];
    outputBuffer[tid.x] = *p;
}

// The pointer element survives in the parameter block as-is.
// CHECK: StructuredBuffer<float *> roBuf_{{[0-9]+}};
// The load is left exactly as found — an ordinary .Load call.
// CHECK: *globalParams_{{[0-9]+}}->roBuf_{{[0-9]+}}.Load(
// CHECK-NOT: __ldg
