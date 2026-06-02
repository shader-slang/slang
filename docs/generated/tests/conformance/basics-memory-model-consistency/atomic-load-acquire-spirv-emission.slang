//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#memory-order
//META: doc_section_digest=f82c5b23c187c6975c3756a23c69a4c91ddfdef1ddfe031714a5cf40822c89c8
//META: purpose=`Atomic<T>.load` with `MemoryOrder.Acquire` emits `OpAtomicLoad` with Acquire semantics in SPIRV, confirming loads/stores after the operation cannot be reordered before it.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "Acquire memory order — loads and stores after a load-acquire
// operation cannot be reordered before the operation."
// In SPIRV, OpAtomicLoad with Acquire semantics has a non-zero semantics operand
// containing the Acquire bit (2) plus UniformMemory (64) = 66 for Device scope.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<Atomic<uint>> atomicBuf;
RWStructuredBuffer<uint> outBuf;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = atomicBuf[0].load(MemoryOrder.Acquire);
}

// CHECK: OpAtomicLoad
// Semantics must include Acquire bit — value is not 0 (relaxed/None).
// CHECK-NOT: %uint_0
