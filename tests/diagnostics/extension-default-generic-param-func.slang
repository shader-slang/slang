//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute -experimental-feature

// The same rule as for `extension` applies to `__func_extension`: its generic
// parameters are inferred from the target function expression, so a default
// value on one of them is meaningless and is rejected.

T cube<T : IArithmetic>(T x)
{
    return x * x * x;
}

struct CubeContext<T : IFloat>
    where T.Differential == T
{
    T cached_sqr;
    void operator()(out T.Differential dx, T.Differential dOut)
    {
        dx = __slang_noop_cast<T.Differential, T>(
            __slang_noop_cast<T, T.Differential>(dOut) * T(3.0) * cached_sqr);
    }
};

__func_extension<T : IFloat = float> __apply(cube<T>)(T x) -> Tuple<T, CubeContext<T>>
//CHECK:                      ^^^^^ default value on extension generic parameter
//CHECK:                      ^^^^^ an 'extension' cannot provide a default value for its generic parameter 'T'
    where T.Differential == T
{
    let sqr = x * x;
    return makeTuple(sqr * x, CubeContext<T>(sqr));
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0.0f;
}
