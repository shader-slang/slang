// Atomics on RWStructuredBuffer<Atomic<T>> elements must be accepted as atomic
// destinations on the LLVM CPU targets: on that path resource types are lowered
// to concrete host-memory pointers before atomic validation runs, and a buffer
// element is a valid destination there, just like a `T*` or `groupshared`
// destination — the groupshared atomic below exercises the latter shape.
// Atomic code generation is not implemented for the LLVM CPU targets, so the
// expected outcome of the first three runs is the ordinary "atomic operations
// are not supported" error — and never the "invalid atomic destination" error
// that would mean validation rejected the destination. Those runs cover both
// target spellings that route through LLVM (-target llvm-host-ir always does;
// -target host-callable only with -emit-cpu-via-llvm) plus one
// -minimum-slang-optimization combination, which is orthogonal to the target
// spelling and pins that the diagnostic still fires when the general
// checkUnsupportedInst pass is skipped there. The last run compiles the same
// shader through the C++ emitter, where atomics ARE supported, to pin that
// nothing outside the CPU-via-LLVM path rejects them. Once the LLVM CPU
// targets implement atomic operations (#14191), this test should become a
// full compute comparison. The CHECK-NOT guards the output up to the first
// E55219, which suffices: validateAtomicOperations runs before
// checkUnsupportedLLVMCPUAtomics, so an E41403 would always print before any
// E55219. Every atomic in the kernel is diagnosed, and the C++ run emits one
// _slang_atomic_add_u32 call per destination.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target llvm-host-ir -emit-cpu-via-llvm -whole-program -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target host-callable -emit-cpu-via-llvm -whole-program -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target host-callable -emit-cpu-via-llvm -whole-program -entry main -stage compute -minimum-slang-optimization
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry main -stage compute -o -

RWStructuredBuffer<Atomic<uint>> counts;
groupshared Atomic<uint> scratch[8];

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint3 t : SV_DispatchThreadID)
{
    counts[t.x & 7].add(1u);
    scratch[t.x & 7].add(1u);
}

//CHECK-NOT: E41403
//CHECK-COUNT-2: error[E55219]: atomic operations are not supported on CPU targets compiled through LLVM
//CPP-NOT: error[E
//CPP-COUNT-2: _slang_atomic_add_u32
