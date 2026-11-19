// Negative companion to no-diff-carry-readnone-derivative-apply.slang and
// -propagate.slang: when the primary, its `__func_extension __apply` and the
// context's `operator()` are all `[__readNone]`, a call whose arguments do not
// carry is itself not carrying, so storing its result into a `no_diff` slot
// compiles clean.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main -experimental-feature

// CHECK: result code = 0
// CHECK-NOT: error[E41031]
// CHECK-NOT: derivative is lost
// CHECK: void main(

RWStructuredBuffer<float> outBuf;

[__readNone]
float cube(float x)
{
    return x * x * x;
}

public struct CubeContext
{
    public float sqr;

    [__readNone]
    public void operator()(out float dx, float dOut)
    {
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
public void callerClean(no_diff float x, no_diff out float result)
{
    result = cube(x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float r;
    callerClean(2.0, r);
    outBuf[0] = r;
}
