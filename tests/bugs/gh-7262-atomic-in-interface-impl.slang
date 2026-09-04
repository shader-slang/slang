//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK-HLSL): -target hlsl
//TEST:SIMPLE(filecheck=CHECK-GLSL): -target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK-CUDA): -target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK-METAL): -target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK-WGSL): -target wgsl -entry main -stage compute

// Regression test: a struct containing Atomic<T> that conforms to an interface
// with a [mutating] method must not be rejected by witness-signature matching.
// The interface's abstract `this` type is copyable (no NonCopyableTypeAttribute),
// while the impl's concrete `this` contains Atomic<T> and gets Ref semantics —
// the two sides must still be accepted as a valid conformance.
//
// Multi-target: the BorrowInOut→Ref promotion is target-independent (shared IR
// lowering), so this test covers SPIRV, HLSL, GLSL, and CUDA to catch regressions
// in any backend's handling of the Ref-parameter type.

//CHECK: OpEntryPoint
// Verify the atomic target is in StorageBuffer address space, not a Function-local copy.
//CHECK-NOT: OpTypePointer Function %int
//CHECK-DAG: OpTypePointer StorageBuffer
//CHECK: OpAtomicIAdd
//CHECK-HLSL: InterlockedAdd
//CHECK-GLSL: atomicAdd
//CHECK-CUDA: atomicAdd
//CHECK-METAL: atomic_fetch_add_explicit
//CHECK-WGSL: atomicAdd

interface ICounter {
    [mutating] void add(int delta);
}

struct AtomicCounter : ICounter {
    Atomic<int> val;

    [mutating]
    void add(int delta) {
        this.val.add(delta);
    }
}

RWStructuredBuffer<AtomicCounter> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {
    buf[0].add(1);
}
