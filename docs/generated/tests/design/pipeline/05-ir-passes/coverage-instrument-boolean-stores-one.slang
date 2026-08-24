//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#instrumentation
//META: doc_section_digest=138b9939ba5bbb4e52a45b224e1ac3fdd87ee9defa3e1c0614c72903418b616e
//META: purpose=-trace-coverage-boolean records execution as a non-atomic store of 1 instead of an exact count.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the alternative recording mode, which is the branch that makes
// the Metal width exemption below it coherent. The same shader compiled
// with counting mode emits an atomic add; here the CHECKs assert the
// counter slot is written with a plain assignment of 1 and that no atomic
// add survives at all, which is the "stores rather than accumulates"
// distinction the row draws.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main -trace-coverage -trace-coverage-boolean

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = int(tid.x);
}

// HLSL: {{.*}}slang_coverage{{.*}}[int(0)] = 1ULL;
// HLSL-NOT: InterlockedAdd
