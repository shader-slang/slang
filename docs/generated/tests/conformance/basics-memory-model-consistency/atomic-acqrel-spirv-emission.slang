//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#memory-order
//META: doc_section_digest=f82c5b23c187c6975c3756a23c69a4c91ddfdef1ddfe031714a5cf40822c89c8
//META: purpose=`Atomic<T>.load` with `MemoryOrder.AcquireRelease` emits `OpAtomicLoad` with AcquireRelease semantics in SPIRV, confirming memory accesses cannot cross the operation.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `Atomic<T>.load` with `MemoryOrder.AcquireRelease` emits `OpAtomicLoad`
// with the AcquireRelease memory semantics bit set (value 72 = AcquireRelease|UniformMemory).
// CHECK pins %uint_72 on the OpAtomicLoad operand, confirming the compiler encodes
// acquire-release ordering on the load — even though SPIRV validity rules disallow it
// (tracked separately; -skip-spirv-validation lets the CHECK observation proceed).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -skip-spirv-validation
//TEST:SIMPLE(filecheck=METALCHECK):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSLCHECK):-target wgsl -entry computeMain -stage compute

RWStructuredBuffer<Atomic<uint>> atomicBuf;
RWStructuredBuffer<uint> outBuf;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = atomicBuf[0].load(MemoryOrder.AcquireRelease);
}

// Compiler emits OpAtomicLoad with semantics 72 = AcquireRelease(8)|UniformMemory(64).
// This is the documented behavior; SPIRV validity is tracked as a separate finding.
// CHECK: OpAtomicLoad
// CHECK-SAME: %uint_72

// Metal: `Atomic<uint>` maps to `atomic<uint>` and the ordered load is emitted as
// atomic_load_explicit with memory_order_seq_cst — Metal has no separate
// acquire-release ordering, so the closest ordering that still forbids memory
// accesses from crossing the operation is seq_cst.
// METALCHECK: atomic_load_explicit
// METALCHECK-SAME: memory_order_seq_cst

// WGSL: `Atomic<uint>` maps to `atomic<u32>` and the load is emitted as atomicLoad.
// WGSL has no per-operation ordering parameter; every WGSL atomic is sequentially
// consistent, which subsumes the acquire-release constraint.
// WGSLCHECK: atomicLoad

// Targets not covered by emission directives (see README `## Untested claims`):
//   hlsl   - `Atomic<T>.load` lowers to a plain RWStructuredBuffer read; HLSL has no
//            atomic-load intrinsic and no memory-order operand to pin.
//   glsl   - Same: plain SSBO element read, no ordering operand on this emit path.
//   cuda   - Plain pointer dereference; the ordering operand is not preserved.
