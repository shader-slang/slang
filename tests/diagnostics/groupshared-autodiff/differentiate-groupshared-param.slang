// A derivative that autodiff generates for a function with a `groupshared` parameter works on a
// private copy of the shared memory, which races with other invocations. We report each call that
// has autodiff generate such a derivative: a `fwd_diff` or `bwd_diff` of the function, and a call
// to it outside `no_diff` from a body that autodiff differentiates, including through an
// interface requirement. Declaring such a function, calling it from code that is not
// differentiated, and calling it under `no_diff` are not reported. The body of such a function is
// never differentiated, so calls in it are not reported either.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;
groupshared float g[4];

[Differentiable]
float helper(no_diff groupshared float a, float x)
//CHECK:                               ^ declare the groupshared variable as a global and access it directly instead of passing it as a parameter
{
    return a * x;
}

// Not reported: `mid` is never differentiated, so neither is `helper` through it.
[Differentiable]
float mid(no_diff groupshared float a, float x)
//CHECK:                            ^ declare the groupshared variable as a global and access it directly instead of passing it as a parameter
//CHECK:                            ^ declare the groupshared variable as a global and access it directly instead of passing it as a parameter
{
    return helper(a, x);
}

interface IScale
{
    [Differentiable]
    float scale(groupshared float a[4], uint i, float x);
//CHECK:                          ^ declare the groupshared variable as a global and access it directly instead of passing it as a parameter
}

struct Scale : IScale
{
    [Differentiable]
    float scale(groupshared float a[4], uint i, float x)
    {
        return a[i] * x;
    }
}

[Differentiable]
float viaInterface<T : IScale>(T t, uint i, float x)
{
    return t.scale(g, i, x);
//CHECK:     ^^^^^ cannot differentiate 'scale' because its parameter 'a' is 'groupshared'
//CHECK:     ^^^^^ a derivative of 'scale' is generated here
}

// The elements passed under `no_diff` are a separate race.
[Differentiable]
float f(float x, uint j)
{
    float r = mid(g[j], x);
//CHECK:      ^^^ cannot differentiate 'mid' because its parameter 'a' is 'groupshared'
//CHECK:      ^^^ a derivative of 'mid' is generated here
    return r + no_diff(helper(g[0], x)) + no_diff(mid(g[1], x));
//CHECK:                       ^ passing part of thread-group-shared memory by reference introduces a race condition when this function is differentiated
//CHECK:                       ^ move the access into a function that is not differentiable or has a custom derivative
//CHECK:                                               ^ passing part of thread-group-shared memory by reference introduces a race condition when this function is differentiated
//CHECK:                                               ^ move the access into a function that is not differentiable or has a custom derivative
}

// Not reported: outside differentiated code.
float plain(uint j, float x)
{
    return mid(g[j], x);
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    var x = diffPair(3.0);
    bwd_diff(mid)(g[i], x, 1.0);
/*CHECK:
    ^^^^^^^^ cannot differentiate 'mid' because its parameter 'a' is 'groupshared'
    ^^^^^^^^ a derivative of 'mid' is generated here
*/
    float d = fwd_diff(helper)(g[i], diffPair(1.0, 1.0)).d;
//CHECK:      ^^^^^^^^ cannot differentiate 'helper' because its parameter 'a' is 'groupshared'
//CHECK:      ^^^^^^^^ a derivative of 'helper' is generated here
    Scale s;
    outputBuffer[i] = x.d + d + plain(i, 1.0) + f(1.0, i) + viaInterface(s, i, 1.0);
}
