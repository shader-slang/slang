// A generic `[__readNone]` primary whose `[BackwardDerivative]` writes a buffer
// must still be treated as carrying. The derivative annotations of a generic
// primary live on the `IRSpecialize` at the call site, so
// `isReadNoneCalleeAndAllDerivatives` has to look them up there rather than on
// the resolved inner function, or this E41031 is silently suppressed.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: error[E41031]
// CHECK: derivative is lost

RWStructuredBuffer<float> debugBuf;
RWStructuredBuffer<float> outBuf;

void squareBwd<T : __BuiltinFloatingPointType>(inout DifferentialPair<T> dpx, T.Differential dOut)
{
    debugBuf[0] = 1.0;
    dpx = diffPair(
        dpx.p,
        __slang_noop_cast<T.Differential>(T(2) * dpx.p * __slang_noop_cast<T>(dOut)));
}

[__readNone]
[BackwardDerivative(squareBwd)]
T square<T : __BuiltinFloatingPointType>(T x)
{
    return x * x;
}

[Differentiable]
public void callerGeneric(no_diff float x, no_diff out float result)
{
    result = square(x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float r;
    callerGeneric(2.0, r);
    outBuf[0] = r;
}
