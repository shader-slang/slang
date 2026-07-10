//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Regression test: a struct containing two fields that are different instantiations of
// the same generic struct, where only the second instantiation transitively contains
// Atomic<T>, must still be detected as non-copyable.
//
// The visited set in typeContainsNonCopyableImpl is keyed on StructDecl*, which is shared
// across all instantiations of a generic struct. Without remove-on-exit semantics, visiting
// Wrapper<int> first would mark the StructDecl as visited and skip Wrapper<Atomic<int>>,
// causing the parent struct to be treated as copyable and producing invalid SPIR-V.

//CHECK: OpEntryPoint
//CHECK-NOT: OpTypePointer Function %int
//CHECK-DAG: OpTypePointer StorageBuffer
//CHECK: OpAtomicIAdd

struct Wrapper<T> { T val; }

struct Parent {
    Wrapper<int> plain;
    Wrapper<Atomic<int>> atomic;

    [mutating]
    void add(int delta) {
        this.atomic.val.add(delta);
    }
}

RWStructuredBuffer<Parent> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {
    buf[0].add(1);
}
