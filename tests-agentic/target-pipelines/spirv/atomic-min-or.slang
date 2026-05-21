//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=InterlockedMin and InterlockedOr on uint lower to OpAtomicUMin and OpAtomicOr respectively — additional members of the validateAtomicOperations-accepted atomic family.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: covers two more atomic opcodes accepted by
// validateAtomicOperations besides the documented IAdd / CAS:
// OpAtomicUMin for InterlockedMin on uint, and OpAtomicOr for
// InterlockedOr.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    InterlockedMin(outBuf[0], 5u);
    InterlockedOr(outBuf[1], 0xFFu);
}

// CHECK-DAG: OpAtomicUMin
// CHECK-DAG: OpAtomicOr
