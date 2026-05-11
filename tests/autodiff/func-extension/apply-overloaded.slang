//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension __apply disambiguating an overloaded function.

// Note: NOT [Differentiable] -- derivatives come solely from __func_extension.
float myFunc(float x) { return x * x; }
float myFunc(float x, float y) { return x * y; }

struct SquareContext
{
    float x;
    void operator()(out float dx, float dOut)
    {
        dx = dOut * 2.0 * x;
    }
};

struct MulContext
{
    float x;
    float y;
    void operator()(out float dx, out float dy, float dOut)
    {
        dx = dOut * y;
        dy = dOut * x;
    }
};

// Custom forward pass for the single-arg overload.
__func_extension __apply(myFunc)(float x) -> Tuple<float, SquareContext>
{
    return makeTuple(myFunc(x), SquareContext(x));
}

// Custom forward pass for the two-arg overload.
__func_extension __apply(myFunc)(float x, float y) -> Tuple<float, MulContext>
{
    return makeTuple(myFunc(x, y), MulContext(x, y));
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Single arg: d/dx myFunc(3.0) = 6.0.
    var dpx = diffPair(3.0, 0.0);
    bwd_diff(myFunc)(dpx, 1.0);
    outputBuffer[0] = dpx.p;
    outputBuffer[1] = dpx.d;

    // Two args: d/dx myFunc(3.0, 4.0) = 4.0, d/dy = 3.0.
    var dpxy = diffPair(3.0, 0.0);
    var dpy = diffPair(4.0, 0.0);
    bwd_diff(myFunc)(dpxy, dpy, 1.0);
    outputBuffer[2] = dpxy.d;
    outputBuffer[3] = dpy.d;

    // CHECK: 3.0
    // CHECK: 6.0
    // CHECK: 4.0
    // CHECK: 3.0
}
