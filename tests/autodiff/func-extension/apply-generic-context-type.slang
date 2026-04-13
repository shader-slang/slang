//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension apply with a fully generic context type
// parameterized by the same type as the func extension.

T cube<T : IArithmetic>(T x) { return x * x * x; }

struct CubeContext<T : IFloat>
    where T.Differential == T
{
    T cached_sqr;
    void operator()(out T.Differential dx, T.Differential dOut)
    {
        dx = __slang_noop_cast<T.Differential, T>(__slang_noop_cast<T, T.Differential>(dOut) * T(3.0) * cached_sqr);
    }
};

__func_extension<T : IFloat> apply(cube<T>)(T x) -> Tuple<T, CubeContext<T>>
    where T.Differential == T
{
    let sqr = x * x;
    return makeTuple(sqr * x, CubeContext<T>(sqr));
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // cube(2.0) = 8.0, d/dx = 3 * 2^2 = 12.0
    var dpx = diffPair(2.0f, 0.0f);
    bwd_diff(cube<float>)(dpx, 1.0f);
    outputBuffer[0] = dpx.d;

    // cube(5.0) = 125.0, d/dx = 3 * 5^2 = 75.0
    var dpy = diffPair(5.0f, 0.0f);
    bwd_diff(cube<float>)(dpy, 1.0f);
    outputBuffer[1] = dpy.d;

    // CHECK: 12.0
    // CHECK: 75.0
}
