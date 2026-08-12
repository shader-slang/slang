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
//CHECK:                      ^^^^^ an extension's generic parameter 'T' is inferred from its target, so it cannot have a default value
    where T.Differential == T
{
    let sqr = x * x;
    return makeTuple(sqr * x, CubeContext<T>(sqr));
}

// The diagnostic is gated on the FuncExtensionDecl, not on the target shape, so
// it fires identically for the `fwd_diff` and `bwd_diff` target shapes too.
T sqr<T : IArithmetic>(T x)
{
    return x * x;
}

__func_extension<T : IFloat = float> fwd_diff(sqr<T>)(DifferentialPair<T> dpx) -> DifferentialPair<T>
//CHECK:                      ^^^^^ default value on extension generic parameter
//CHECK:                      ^^^^^ an extension's generic parameter 'T' is inferred from its target, so it cannot have a default value
    where T.Differential == T
{
    return diffPair(dpx.p, dpx.d);
}

__func_extension<T : IFloat = float> bwd_diff(sqr<T>)(inout DifferentialPair<T> dpx, T dOut) -> void
//CHECK:                      ^^^^^ default value on extension generic parameter
//CHECK:                      ^^^^^ an extension's generic parameter 'T' is inferred from its target, so it cannot have a default value
    where T.Differential == T
{
    dpx = diffPair(dpx.p, dOut);
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0.0f;
}
