//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#atomics
//META: doc_section_digest=a51e4f61c144b6b055f62cb6dd435450ea8054e86989ebdfc212ed4b29838fc8
//META: purpose=`Atomic<T>.store` with `MemoryOrder.Relaxed` emits `OpAtomicStore` with None (0) memory semantics in SPIRV, confirming no ordering constraint beyond atomicity.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "The relaxed memory ordering with atomic memory access does
// not provide any ordering relationship with other memory accesses. All that
// is guaranteed is that the modifications are atomic and the modifications
// occur in a total order specific to the variable."
// SPIRV: OpAtomicStore with memory semantics operand = 0 (None/Relaxed).
// %uint_0 is the semantics operand in the store below.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<Atomic<uint>> atomicBuf;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    atomicBuf[0].store(42u, MemoryOrder.Relaxed);
}

// CHECK: OpAtomicStore
// CHECK-SAME: %uint_0
