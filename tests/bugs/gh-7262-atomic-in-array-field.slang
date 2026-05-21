//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Regression test: [mutating] method on a struct whose field is Atomic<T>[N]
// (an array of atomics) must also use Ref semantics, not BorrowInOut.
// Exercises the ArrayExpressionType branch in typeContainsNonCopyable.

//CHECK: OpEntryPoint
//CHECK: OpAtomicIIncrement

struct TestArray {
    Atomic<int> counts[4];

    [mutating]
    void increment(int i) {
        this.counts[i].increment();
    }
}

RWStructuredBuffer<TestArray> data;

[shader("compute")]
[numthreads(1, 1, 1)]
public void main() {
    data[0].increment(1);
}
