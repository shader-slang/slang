//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Regression test: [mutating] method on a generic struct whose Atomic<T> field
// is instantiated with a concrete T must use true-reference (Ref) semantics.
// Exercises the generic-substitution path in typeContainsNonCopyable: the raw
// field type is Atomic<T>, which after substituting T=int becomes Atomic<int>.

//CHECK: OpEntryPoint
// Verify the atomic target is in StorageBuffer address space, not a Function-local copy.
//CHECK-NOT: OpTypePointer Function
//CHECK-DAG: OpTypePointer StorageBuffer
//CHECK: OpAtomicExchange

struct Wrapper<T : IAtomicable> {
    Atomic<T> val;

    [mutating]
    void update(T newVal) {
        this.val.exchange(newVal);
    }
}

RWStructuredBuffer<Wrapper<int>> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {
    buf[0].update(42);
}
