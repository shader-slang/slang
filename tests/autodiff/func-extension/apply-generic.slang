//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension apply with generic specialization

T sqr<T : IArithmetic>(T x) { return x * x; }

struct SqrContextF
{
    float cached_x;
    void operator()(out float dx, float dOut)
    {
        dx = dOut * 2.0 * cached_x;
    }
};

// apply only for float specialization
__func_extension apply(sqr<float>)(float x) -> Tuple<float, SqrContextF>
{
    return makeTuple(x * x, SqrContextF(x));
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // sqr(4.0) = 16.0, d/dx = 2*4 = 8.0
    var dpx = diffPair(4.0, 0.0);
    bwd_diff(sqr<float>)(dpx, 1.0);
    outputBuffer[0] = dpx.d;
    // CHECK: 8.0
}
