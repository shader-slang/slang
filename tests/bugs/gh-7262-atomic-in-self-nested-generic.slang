//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Regression test: a generic struct nested inside another instantiation of ITSELF
// (e.g. Wrapper<Wrapper<Atomic<int>>>) must still be detected as transitively
// containing a non-copyable field.
//
// A StructDecl*-keyed visited set cannot handle this case: the outer and inner
// instantiations of Wrapper share the same StructDecl*, so a visited-set check
// would short-circuit the inner traversal and miss the Atomic<int> field entirely,
// causing BorrowInOut (copy-in/copy-out) semantics instead of Ref semantics and
// producing invalid SPIR-V.

//CHECK: OpEntryPoint
// Verify the atomic target is in StorageBuffer address space, not a Function-local copy.
//CHECK-NOT: OpTypePointer Function %int
//CHECK-DAG: OpTypePointer StorageBuffer
//CHECK: OpAtomicIAdd

struct Wrapper<T> { T val; }

struct Container {
    Wrapper<Wrapper<Atomic<int>>> nested;

    [mutating]
    void add(int delta) {
        this.nested.val.val.add(delta);
    }
}

RWStructuredBuffer<Container> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {
    buf[0].add(1);
}
