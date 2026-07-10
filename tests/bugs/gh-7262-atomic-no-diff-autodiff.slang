//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Regression test: getParamPassingMode must use getType (substituted declared type)
// rather than getParamValueType. getParamValueType wraps no_diff parameters in
// ModifiedType(NoDiffModifierVal,...), which breaks isCopyableType/isNonCopyableType
// since those do not unwrap ModifiedType. Without the fix, a `no_diff Atomic<int>`
// member in an IDifferentiable struct would be misidentified as copyable, its
// In-mode would not be promoted to BorrowIn, and a copy-in would be attempted
// for a non-copyable field. The imaginary-args derivative helpers
// (getImaginaryArgsToForwardDerivative / getImaginaryArgsToBackwardDerivative) both
// call getParamPassingMode and exercise this path.

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
