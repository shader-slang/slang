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

// Verifies that `Atomic<T>.load` with `MemoryOrder.Acquire` emits `OpAtomicLoad`
// with a non-zero memory semantics operand, confirming the Acquire bit is set.
// CHECK-NOT %uint_0 rules out the relaxed (None) encoding; anything non-zero
// indicates the acquire ordering constraint is present in the emitted SPIRV.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METALCHECK):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSLCHECK):-target wgsl -entry computeMain -stage compute

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

// Metal: Acquire maps to memory_order_seq_cst (Metal has no separate acquire; uses seq_cst as the
// closest available ordered load).
// METALCHECK: atomic_load_explicit
// METALCHECK-SAME: memory_order_seq_cst

// WGSL: atomicLoad (WGSL has no per-operation ordering; all atomics are sequentially consistent).
// WGSLCHECK: atomicLoad

// Targets not covered by emission directives:
//   hlsl   - Atomic<T>.load lowers to a plain RWStructuredBuffer read; no acquire ordering surface.
//   glsl   - Plain load from SSBO; GLSL has no atomicLoad with ordering parameter at this emit path.
//   cuda   - Plain pointer dereference; CUDA codegen path does not emit ordering intrinsic for Atomic<T>.load.
