//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension apply(fn) — custom forward pass with context
// Note: cube is NOT [Differentiable] — the derivative comes solely
// from the __func_extension apply.

float cube(float x) { return x * x * x; }

struct CubeContext
{
    float sqr;
    void operator()(out float dx, float dOut)
    {
        dx = dOut * 3.0 * sqr;
    }
};

__func_extension apply(cube)(float x) -> Tuple<float, CubeContext>
{
    let sqr = x * x;
    return makeTuple(sqr * x, CubeContext(sqr));
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Backward: d/dx(cube) at x=2.0 with dOut=1.0 should be 3*4 = 12.0
    var dpx = diffPair(2.0, 0.0);
    bwd_diff(cube)(dpx, 1.0);
    outputBuffer[0] = dpx.d;
    // CHECK: 12.0
}
