//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0e5b4c31fb3cb504b87417d8391d1c84f2175e1be41f81660fa7556568fce3ac
//META: purpose=InterlockedCompareExchange lowers to OpAtomicCompareExchange; validateAtomicOperations accepts the CAS form alongside the documented OpAtomicIAdd.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: the doc references validateAtomicOperations inside
// legalizeIRForSPIRV as the gatekeeper for atomic ops. The bundle
// pins the OpAtomicIAdd lowering for InterlockedAdd; this test
// covers the CAS variant — InterlockedCompareExchange lowers to
// OpAtomicCompareExchange.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    uint orig;
    InterlockedCompareExchange(outBuf[0], 0u, 5u, orig);
    outBuf[1] = orig;
}

// CHECK: OpAtomicCompareExchange
