//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#memory-order
//META: doc_section_digest=f82c5b23c187c6975c3756a23c69a4c91ddfdef1ddfe031714a5cf40822c89c8
//META: purpose=`Atomic<T>.store` with `MemoryOrder.Release` emits `OpAtomicStore` with Release semantics in SPIRV, confirming loads/stores before the operation cannot be reordered after it.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "Release memory order — loads or stores before a store-release
// operation cannot be reordered after the operation."
// In SPIRV, OpAtomicStore with Release semantics has a semantics operand
// containing the Release bit (4) plus UniformMemory (64) = 68.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<Atomic<uint>> atomicBuf;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    atomicBuf[0].store(1u, MemoryOrder.Release);
}

// CHECK: OpAtomicStore
// Semantics operand for Release is 68 = UniformMemory(64) + Release(4).
// CHECK-SAME: %uint_68
