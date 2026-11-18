// Atomics on RWStructuredBuffer<Atomic<T>> elements must be accepted as atomic
// destinations on the LLVM CPU target: on that path resource types are lowered
// to concrete host-memory pointers before atomic validation runs, and a buffer
// element qualifies as device-buffer memory just like a `T*` or `groupshared`
// destination does. The LLVM emitter does not implement atomic instructions
// yet (tracked as #14191, whose implementer should upgrade this test to a
// full compute comparison), so the expected outcome of this compile is the
// unsupported-instruction diagnostic — and never the
// invalid-atomic-destination diagnostic.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target llvm-host-ir -emit-cpu-via-llvm -whole-program -entry main -stage compute

RWStructuredBuffer<Atomic<uint>> counts;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint3 t : SV_DispatchThreadID)
{
    counts[t.x & 7].add(1u);
}

// CHECK-NOT: E41403
// CHECK: error[E99997]
