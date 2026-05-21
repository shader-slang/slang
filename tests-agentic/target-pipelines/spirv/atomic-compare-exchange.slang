//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
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
