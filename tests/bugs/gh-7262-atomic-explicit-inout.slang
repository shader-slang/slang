//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Regression test: an explicit `inout` parameter whose struct contains Atomic<T>
// is also promoted to Ref semantics, because `inout` maps to BorrowInOut mode in
// getExplicitlyDeclaredParamPassingMode — the same mode as `[mutating] this`.
// The BorrowInOut→Ref promotion in adjustParamPassingModeBasedOnParamType therefore
// applies to both cases.

//CHECK: OpEntryPoint
//CHECK-NOT: OpTypePointer Function %int
//CHECK-DAG: OpTypePointer StorageBuffer
//CHECK: OpAtomicIIncrement

struct W {
    Atomic<int> a;
}

void bump(inout W w) {
    w.a.increment();
}

RWStructuredBuffer<W> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {
    bump(buf[0]);
}
