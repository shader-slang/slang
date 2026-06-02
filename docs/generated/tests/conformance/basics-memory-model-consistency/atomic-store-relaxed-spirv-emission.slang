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

// Verifies that `Atomic<T>.store` with `MemoryOrder.Relaxed` emits `OpAtomicStore`
// with memory semantics operand 0 (None) in SPIRV, confirming no ordering constraint
// is attached. CHECK-SAME pins %uint_0 immediately after OpAtomicStore, which is the
// semantics operand position — proving the relaxed encoding is used.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<Atomic<uint>> atomicBuf;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    atomicBuf[0].store(42u, MemoryOrder.Relaxed);
}

// CHECK: OpAtomicStore
// CHECK-SAME: %uint_0
