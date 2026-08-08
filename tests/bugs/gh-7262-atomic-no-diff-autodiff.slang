//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Regression test: a [Differentiable] function whose struct parameter contains a
// no_diff Atomic<int> field must still emit a valid atomic SPIR-V op rather than a
// Function-space pointer. The struct Counted implements IDifferentiable and has a
// no_diff Atomic<int> counter; calling compute (which mutates counter via add())
// must lower to OpAtomicIAdd, not to a Function-space copy.

//CHECK: OpEntryPoint
//CHECK-NOT: OpTypePointer Function %int
//CHECK-DAG: OpTypePointer StorageBuffer
//CHECK: OpAtomicIAdd

struct Counted : IDifferentiable {
    no_diff Atomic<int> counter;
    float val;
}

[Differentiable]
float compute(no_diff RWStructuredBuffer<Counted> buf, float x) {
    buf[0].counter.add(1);
    return buf[0].val * x;
}

RWStructuredBuffer<Counted> g_buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {
    float result = compute(g_buf, 2.0f);
}
