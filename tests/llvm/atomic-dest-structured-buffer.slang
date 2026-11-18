// Atomics on RWStructuredBuffer<Atomic<T>> elements must be accepted as atomic
// destinations on the LLVM CPU targets: on that path resource types are lowered
// to concrete host-memory pointers before atomic validation runs, and a buffer
// element is a valid destination there, just like a `T*` or `groupshared`
// destination. Atomic code generation is not implemented for the LLVM CPU
// targets, so the expected outcome of this compile is the ordinary
// "atomic operations are not supported" error — and never the
// "invalid atomic destination" error that would mean validation rejected the
// destination. Both CPU target spellings below reach the same CPU-via-LLVM
// pipeline; once the LLVM CPU targets implement atomic operations (#14191),
// this test should become a full compute comparison.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target llvm-host-ir -emit-cpu-via-llvm -whole-program -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target host-callable -emit-cpu-via-llvm -whole-program -entry main -stage compute

RWStructuredBuffer<Atomic<uint>> counts;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint3 t : SV_DispatchThreadID)
{
    counts[t.x & 7].add(1u);
}

// CHECK-NOT: E41403
// CHECK: error[E55219]
