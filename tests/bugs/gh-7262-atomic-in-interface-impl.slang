//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Regression test: a struct containing Atomic<T> that conforms to an interface
// with a [mutating] method must not be rejected by witness-signature matching.
// The interface's abstract `this` type is copyable (no NonCopyableTypeAttribute),
// while the impl's concrete `this` contains Atomic<T> and gets Ref semantics —
// the two sides must still be accepted as a valid conformance.

//CHECK: OpEntryPoint
//CHECK: OpAtomicIAdd

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
