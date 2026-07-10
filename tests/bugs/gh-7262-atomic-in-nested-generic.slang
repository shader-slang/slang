//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Regression test: a doubly-nested generic where the outer type parameter T
// is substituted into an inner Wrapper<Atomic<T>> field. This exercises the
// SubstitutionSet scope in typeContainsNonCopyableImpl: the field's type is
// initially GenericParamType(T), which must be resolved through the outer
// struct's substitution (T=int) before the inner Wrapper's substitution
// (T'=Atomic<int>) can be applied.

//CHECK: OpEntryPoint
//CHECK-NOT: OpTypePointer Function %int
//CHECK-DAG: OpTypePointer StorageBuffer
//CHECK: OpAtomicExchange

struct Wrapper<T> { T val; }

struct Outer<T : IAtomicable> {
    Wrapper<Atomic<T>> w;

    [mutating]
    void swap(T newVal) {
        this.w.val.exchange(newVal);
    }
}

RWStructuredBuffer<Outer<int>> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {
    buf[0].swap(42);
}
