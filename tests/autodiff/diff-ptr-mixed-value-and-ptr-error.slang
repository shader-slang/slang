//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

// Test that mixing IDifferentiable value types with IDifferentiablePtrType outputs
// in a differentiable function produces an error diagnostic.

struct MyPtr : IDifferentiablePtrType
{
    typealias Differential = MyPtr;
    uint offset;

    [ForwardDerivative(fwd_init)]
    __init(uint x)
    {
        offset = x;
    }

    static DifferentialPtrPair<MyPtr> fwd_init(uint x)
    {
        MyPtr primal = MyPtr(x);
        MyPtr diff = MyPtr(x);
        return DifferentialPtrPair<MyPtr>(primal, diff);
    }
}

// This function has both an IDifferentiablePtrType output (inout MyPtr)
// and an IDifferentiable value input (float x), which is not supported.
[Differentiable]
[ForwardDerivative(fwd_mixed)]
float mixedFunc(inout MyPtr ptr, float x)
{
    return x * ptr.offset;
}

DifferentialPair<float> fwd_mixed(inout DifferentialPtrPair<MyPtr> ptr, DifferentialPair<float> x)
{
    return diffPair(x.p * ptr.p.offset, x.d * ptr.p.offset);
}

[Differentiable]
float caller(float x)
{
    var ptr = MyPtr(2.0);
    let r = mixedFunc(ptr, x);
//CHECK:             ^ cannot mix differentiable value types with differentiable pointer outputs
    return r;
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = fwd_diff(caller)(diffPair(1.0, 1.0)).d;
}
