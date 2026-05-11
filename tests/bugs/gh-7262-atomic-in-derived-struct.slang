//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Regression test: [mutating] method on a struct that *inherits* from a struct
// containing Atomic<T> must also use true-reference (Ref) semantics, not
// copy-in/copy-out. Exercises the InheritanceDecl traversal in typeContainsNonCopyable.

//CHECK: OpEntryPoint
//CHECK: OpAtomicIIncrement

struct AtomicBase {
    Atomic<int> counter;
}

struct Derived : AtomicBase {
    [mutating]
    void increment() {
        this.counter.increment();
    }
}

RWStructuredBuffer<Derived> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {
    buf[0].increment();
}
