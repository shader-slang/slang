// bwd_diff is no longer further differentiable: differentiating `df` (which calls
// `__bwd_diff(outerF)`) must be diagnosed rather than crash, even when the inner function uses a
// `do`/`while` loop. See https://github.com/shader-slang/slang/issues/10745.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage compute -entry computeMain

RWStructuredBuffer<float> outputBuffer;

struct A : IDifferentiable
{
    float x;
    int nx;
}

[BackwardDifferentiable]
float f(A x)
{
    A rs;
    rs.x = 1.0;
    for (int i = 0; i < 2; i++)
        rs.x = rs.x * x.x;
    return rs.x;
}

[BackwardDifferentiable]
float outerF(A x)
{
    A nx;
    nx.x = 1.0;
    int i = 0;
    [MaxIters(3)]
    do
    {
        nx.x = nx.x * x.x;
        i++;
    }
    while (i < 2);
    nx.nx = 2;//x.nx;
    return f(nx);
}

[BackwardDifferentiable]
float df(A x)
{
    A.Differential ad;
    ad.x = 0.0;
    var p = diffPair(x, ad);
    __bwd_diff(outerF)(p, 1.0);
//CHECK:              ^ cannot differentiate the result of a backward-derivative call
//CHECK:              ^ the code produced by 'bwd_diff' is not itself differentiable
    return p.d.x;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Given f(x) = x^4,
    // f''(x) = 12 * x^2
    // Expect f''(4) = 192
    A a;
    a.x = 4.0;
    a.nx = 54;
    A.Differential ad;
    ad.x = 1.0;
    var p = diffPair(a, ad);
    outputBuffer[0] = __fwd_diff(df)(p).d;
}
