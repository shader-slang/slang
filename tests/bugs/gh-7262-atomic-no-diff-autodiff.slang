//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Regression test: an `inout no_diff` parameter whose type transitively contains
// Atomic<T> must still be promoted from copy-in/copy-out (BorrowInOut) to a true
// reference (Ref), and must lower to an atomic op on the caller's storage, not a
// Function-space copy.
//
// This specifically exercises the getType-vs-getParamValueType distinction in
// getParamPassingMode (slang-lower-to-ir.cpp): getParamValueType wraps a `no_diff`
// parameter's type in ModifiedType(NoDiffModifierVal, Counted), but
// adjustParamPassingModeBasedOnParamType's isCopyableType/typeContainsNonCopyable
// checks do not unwrap ModifiedType at their outer call site. getParamPassingMode
// must feed the plain substituted type (via getType), not the ModifiedType-wrapped
// one, or the promotion is silently skipped and the atomic ends up on a local copy.
// A `no_diff` *field* inside an IDifferentiable struct (as opposed to a `no_diff`
// parameter) does not exercise this path, because getParamValueType only wraps
// ModifiedType around parameter types, not field types.

//CHECK: OpEntryPoint
//CHECK-NOT: OpTypePointer Function %int
//CHECK-DAG: %[[SB_INT_PTR:[a-zA-Z0-9_]+]] = OpTypePointer StorageBuffer %int
//CHECK: %[[ATOMIC_DEST:[a-zA-Z0-9_]+]] = OpAccessChain %[[SB_INT_PTR]] {{.*}}
//CHECK: OpAtomicIAdd %int %[[ATOMIC_DEST]]

struct Counted {
    Atomic<int> counter;
}

[Differentiable]
float compute(inout no_diff Counted counter, float x)
{
    counter.counter.add(1);
    return x * x;
}

RWStructuredBuffer<Counted> g_buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float result = compute(g_buf[0], 2.0f);
}
