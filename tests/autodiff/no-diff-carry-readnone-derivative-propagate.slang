// A `[__readNone]` primary whose `__func_extension __apply` context has a
// buffer-writing `operator()` must still be treated as carrying, so the store
// into a `no_diff` slot below is diagnosed. The `__apply` function itself is
// `[__readNone]` and the primary has no `[ForwardDerivative]`, so only the
// `BackwardDerivativePropagate` association of
// `isReadNoneCalleeAndAllDerivatives` can reject the call.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main -experimental-feature

// CHECK: error[E41031]
// CHECK: derivative is lost

RWStructuredBuffer<float> debugBuf;
RWStructuredBuffer<float> outBuf;

[__readNone]
float cube(float x)
{
    return x * x * x;
}

public struct CubeContext
{
    public float sqr;

    public void operator()(out float dx, float dOut)
    {
        debugBuf[0] = dOut;
        dx = dOut * 3.0 * sqr;
    }
};

[__readNone]
__func_extension __apply(cube)(float x) -> Tuple<float, CubeContext>
{
    let sqr = x * x;
    return makeTuple(sqr * x, CubeContext(sqr));
}

[Differentiable]
public void callerPropagate(no_diff float x, no_diff out float result)
{
    result = cube(x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float r;
    callerPropagate(2.0, r);
    outBuf[0] = r;
}
