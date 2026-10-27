// Autodiff prepares a function body for differentiation by replacing the addresses it computes
// with values, so a reference argument that names part of thread-group-shared memory, or the
// function's own `groupshared` parameter, is copied in before the call and stored back after it,
// whatever the reference's access. A whole `groupshared` variable keeps its address, but passing
// it to a writable reference lets a callee that we cannot check write it. Each case below is one
// of these shapes inside a differentiated body or at a `fwd_diff`/`bwd_diff` call. The cases
// that should stay silent are in silent.slang.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute -experimental-feature

RWStructuredBuffer<float> outputBuffer;

struct Pair
{
    float a;
    float b;
}

groupshared float g[4];
groupshared Pair gPair;

[Differentiable]
float scale(no_diff __ref float a, float x)
{
    return a * x;
}

[Differentiable]
float sumAll(no_diff __ref float a[4], float x)
{
    return (a[0] + a[1] + a[2] + a[3]) * x;
}

[Differentiable]
[ForceInline]
float scaleInline(no_diff __ref float a, float x)
{
    return a * x;
}

float bump(__ref float a)
{
    a += 1.0;
    return a;
}

[Differentiable]
float readOnly(no_diff __ref_readonly float a, float x)
{
    return a * x;
}

[Differentiable]
float elementToDifferentiable(uint i, float x)
{
    return scale(g[i], x);
//CHECK:          ^ passing thread-group-shared memory by reference introduces a race condition when this function is differentiated
//CHECK:          ^ access the groupshared variable directly as a global instead of passing it as an argument; if forward or backward differentiation of this function is needed, provide a custom derivative
}

[Differentiable]
float wholeArrayToWritable(float x)
{
    return sumAll(g, x);
//CHECK:          ^ passing thread-group-shared memory by reference introduces a race condition when this function is differentiated
//CHECK:          ^ access the groupshared variable directly as a global in the callee instead of passing it as an argument
}

[Differentiable]
float structField(float x)
{
    return scale(gPair.b, x);
//CHECK:               ^ passing thread-group-shared memory by reference introduces a race condition when this function is differentiated
//CHECK:               ^ access the groupshared variable directly as a global instead of passing it as an argument; if forward or backward differentiation of this function is needed, provide a custom derivative
}

[Differentiable]
float underNoDiff(uint i, float x)
{
    return x * no_diff(bump(g[i]));
//CHECK:                     ^ passing thread-group-shared memory by reference introduces a race condition when this function is differentiated
//CHECK:                     ^ access the groupshared variable directly as a global instead of passing it as an argument; if forward or backward differentiation of this function is needed, provide a custom derivative
}

[Differentiable]
float toForceInlineDifferentiable(uint i, float x)
{
    return scaleInline(g[i], x);
//CHECK:                ^ passing thread-group-shared memory by reference introduces a race condition when this function is differentiated
//CHECK:                ^ access the groupshared variable directly as a global instead of passing it as an argument; if forward or backward differentiation of this function is needed, provide a custom derivative
}

[Differentiable]
float throughReadOnly(uint i, float x)
{
    return readOnly(g[i], x);
//CHECK:             ^ passing thread-group-shared memory by reference introduces a race condition when this function is differentiated
//CHECK:             ^ access the groupshared variable directly as a global instead of passing it as an argument; if forward or backward differentiation of this function is needed, provide a custom derivative
}

[Differentiable]
float forwardOwnGroupshared(no_diff groupshared float a, float x)
{
    return readOnly(a, x);
//CHECK:            ^ passing thread-group-shared memory by reference introduces a race condition when this function is differentiated
//CHECK:            ^ access the groupshared variable directly as a global instead of passing it as an argument; if forward or backward differentiation of this function is needed, provide a custom derivative
}

[ForwardDifferentiable]
float forwardModeOnly(uint i, float x)
{
    return scale(g[i], x);
//CHECK:          ^ passing thread-group-shared memory by reference introduces a race condition when this function is differentiated
//CHECK:          ^ access the groupshared variable directly as a global instead of passing it as an argument; if forward differentiation of this function is needed, provide a custom derivative
}

[Differentiable]
[ForwardDerivative(customForwardOnlyFwd)]
float customForwardOnly(uint i, float x)
{
    return scale(g[i], x);
//CHECK:          ^ passing thread-group-shared memory by reference introduces a race condition when this function is differentiated
//CHECK:          ^ access the groupshared variable directly as a global instead of passing it as an argument; if backward differentiation of this function is needed, provide a custom derivative
}

DifferentialPair<float> customForwardOnlyFwd(uint i, DifferentialPair<float> x)
{
    return diffPair(g[i] * x.p, g[i] * x.d);
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    var x = diffPair(3.0);
    bwd_diff(sumAll)(g, x, 1.0);
//CHECK:             ^ passing thread-group-shared memory by reference introduces a race condition when this call is differentiated
//CHECK:             ^ access the groupshared variable directly as a global in the callee instead of passing it as an argument
    float d = fwd_diff(sumAll)(g, diffPair(1.0, 1.0)).d;
//CHECK:                       ^ passing thread-group-shared memory by reference introduces a race condition when this call is differentiated
//CHECK:                       ^ access the groupshared variable directly as a global in the callee instead of passing it as an argument
    outputBuffer[i] = x.d + d + elementToDifferentiable(i, 1.0) + wholeArrayToWritable(1.0) +
                      structField(1.0) + underNoDiff(i, 1.0) +
                      toForceInlineDifferentiable(i, 1.0) + throughReadOnly(i, 1.0) +
                      forwardOwnGroupshared(g[i], 1.0) + forwardModeOnly(i, 1.0) +
                      customForwardOnly(i, 1.0);
}
