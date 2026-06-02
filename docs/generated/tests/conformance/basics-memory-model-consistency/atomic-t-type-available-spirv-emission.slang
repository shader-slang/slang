//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#atomics
//META: doc_section_digest=a51e4f61c144b6b055f62cb6dd435450ea8054e86989ebdfc212ed4b29838fc8
//META: purpose=The `Atomic<T>` type is available in Slang for atomic memory access and compiles to SPIRV `OpAtomicStore`/`OpAtomicLoad` operations.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `Atomic<T>` is usable as a structured-buffer element type and that
// a store/load round-trip on it emits `OpAtomicStore` + `OpAtomicLoad` in SPIRV.
// Both CHECKs must fire for the test to pass, confirming both operations lower
// to atomic SPIRV instructions rather than plain loads or stores.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSLCHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<Atomic<uint>> atomicBuf;
RWStructuredBuffer<uint> outBuf;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    atomicBuf[0].store(tid.x, MemoryOrder.Relaxed);
    outBuf[0] = atomicBuf[0].load(MemoryOrder.Relaxed);
}

// CHECK: OpAtomicStore
// CHECK: OpAtomicLoad

// HLSL: Atomic<T> elements are accessed via interlocked-style assignment; output
// for the store and load should include the buffer variable.
// HLSLCHECK: atomicBuf
