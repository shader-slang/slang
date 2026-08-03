//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Regression test: a struct containing two fields that are different instantiations of
// the same generic struct, where only the second instantiation transitively contains
// Atomic<T>, must still be detected as non-copyable.
//
// typeContainsNonCopyableImpl bounds recursion with a depth limit (kMaxTypeNestingDepth)
// rather than a StructDecl*-keyed visited set. A StructDecl*-keyed set cannot be used
// here because a generic struct appears at multiple depths with different type arguments —
// e.g. `Wrapper<Atomic<int>>` and `Wrapper<int>` share the same StructDecl* but have
// different fields after substitution. The depth limit avoids this while still bounding
// stack use; valid programs cannot have by-value struct cycles so the limit is only a
// safety guard.

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
