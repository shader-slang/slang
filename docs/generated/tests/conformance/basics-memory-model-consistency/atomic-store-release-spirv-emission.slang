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

// Verifies that `Atomic<T>.store` with `MemoryOrder.Release` emits `OpAtomicStore`
// with semantics operand 68 (Release(4) | UniformMemory(64)) in SPIRV.
// CHECK-SAME pins %uint_68 on the OpAtomicStore line, confirming the Release bit
// is present — distinguishing it from a relaxed (None/0) store.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METALCHECK):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSLCHECK):-target wgsl -entry computeMain -stage compute

RWStructuredBuffer<Atomic<uint>> atomicBuf;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    atomicBuf[0].store(1u, MemoryOrder.Release);
}

// CHECK: OpAtomicStore
// Semantics operand for Release is 68 = UniformMemory(64) + Release(4).
// CHECK-SAME: %uint_68

// Metal: Release maps to memory_order_seq_cst (Metal has no separate release ordering;
// the closest stronger ordering available is seq_cst).
// METALCHECK: atomic_store_explicit
// METALCHECK-SAME: memory_order_seq_cst

// WGSL: atomicStore (WGSL has no per-operation ordering; all atomics are sequentially consistent).
// WGSLCHECK: atomicStore

// Targets not covered by emission directives:
//   hlsl   - Atomic<T>.store(Release) lowers to a plain RWStructuredBuffer assignment; no release ordering surface.
//   glsl   - Plain store to SSBO; no release-ordering parameter in GLSL atomic store path.
//   cuda   - Plain pointer write; CUDA codegen path does not emit release intrinsic for Atomic<T>.store.
