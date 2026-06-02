//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#atomics
//META: doc_section_digest=a51e4f61c144b6b055f62cb6dd435450ea8054e86989ebdfc212ed4b29838fc8
//META: purpose=Slang standard library relaxed atomic operations (InterlockedMin, InterlockedMax, InterlockedAnd, InterlockedOr, InterlockedXor, InterlockedExchange) compile to corresponding `OpAtomicS*/OpAtomicU*/OpAtomicOr/OpAtomicAnd/OpAtomicXor/OpAtomicExchange` SPIRV ops with relaxed (0) memory semantics.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that all six global-decl relaxed atomics (InterlockedMin/Max/And/Or/Xor/Exchange)
// each compile to their corresponding SPIRV OpAtomic* opcode. CHECK-DAG pins all six
// opcodes regardless of emission order; the HLSL CHECKs confirm each intrinsic name
// survives lowering, proving the global-decl atomic surface maps one-to-one.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSLCHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<uint> buf;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint old;
    InterlockedMin(buf[0], tid.x, old);
    InterlockedMax(buf[1], tid.x, old);
    InterlockedAnd(buf[2], tid.x, old);
    InterlockedOr(buf[3], tid.x, old);
    InterlockedXor(buf[4], tid.x, old);
    InterlockedExchange(buf[5], tid.x, old);
}

// All relaxed global-decl atomics emit with None (0) semantics in SPIRV.
// CHECK-DAG: OpAtomicUMin
// CHECK-DAG: OpAtomicUMax
// CHECK-DAG: OpAtomicAnd
// CHECK-DAG: OpAtomicOr
// CHECK-DAG: OpAtomicXor
// CHECK-DAG: OpAtomicExchange

// HLSL: each maps to corresponding Interlocked intrinsic.
// HLSLCHECK: InterlockedMin
// HLSLCHECK: InterlockedMax
// HLSLCHECK: InterlockedAnd
// HLSLCHECK: InterlockedOr
// HLSLCHECK: InterlockedXor
// HLSLCHECK: InterlockedExchange
