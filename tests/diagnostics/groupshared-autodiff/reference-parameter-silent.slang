// Thread-group-shared memory passed to a reference parameter of a differentiable function is
// accessed in place by the callee, so none of these cases is reported: a local or an element to a
// `__ref` parameter, a whole array to a `__ref_readonly` or a `__ref` parameter, a function that
// forwards its own reference parameter, and a write through the function's own `groupshared`
// parameter. Each is differentiated directly or through a differentiable caller.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute -experimental-feature

RWStructuredBuffer<float> outputBuffer;

groupshared float g[4];

[Differentiable]
float scale(no_diff __ref float a, float x)
{
    return a * x;
}

[Differentiable]
float sumAll(no_diff __ref_readonly float a[4], float x)
{
    return (a[0] + a[1] + a[2] + a[3]) * x;
}

[Differentiable]
float sumAllWritable(no_diff __ref float a[4], float x)
{
    return (a[0] + a[1] + a[2] + a[3]) * x;
}

[Differentiable]
float forwardOwnParam(no_diff __ref float a, float x)
{
    return scale(a, x);
}

[Differentiable]
float writeThroughOwnParameter(no_diff groupshared float a[4], uint i, float x)
{
    a[i] = 1.0;
    return a[i] * x;
}

[Differentiable]
float localByReference(float x)
{
    float local = 2.0;
    return forwardOwnParam(local, x);
}

[Differentiable]
float elementToDifferentiable(uint i, float x)
{
    return scale(g[i], x);
}

[Differentiable]
float wholeArrayToReadOnly(float x)
{
    return sumAll(g, x);
}

[Differentiable]
float wholeArrayToWritable(float x)
{
    return sumAllWritable(g, x);
}

[Differentiable]
float callWithGroupSharedParameter(uint i, float x)
{
    return writeThroughOwnParameter(g, i, x);
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    var x = diffPair(3.0);
    bwd_diff(localByReference)(x, 1.0);
    var z = diffPair(3.0);
    bwd_diff(scale)(g[i], z, 1.0);
    float d = fwd_diff(scale)(g[i], diffPair(1.0, 1.0)).d;
    var w = diffPair(3.0);
    bwd_diff(sumAll)(g, w, 1.0);
    var v = diffPair(3.0);
    bwd_diff(writeThroughOwnParameter)(g, i, v, 1.0);
    float e = fwd_diff(sumAllWritable)(g, diffPair(1.0, 1.0)).d;
    outputBuffer[i] = x.d + z.d + d + w.d + v.d + e + elementToDifferentiable(i, 1.0) +
                      wholeArrayToReadOnly(1.0) + wholeArrayToWritable(1.0) +
                      callWithGroupSharedParameter(i, 1.0);
}
