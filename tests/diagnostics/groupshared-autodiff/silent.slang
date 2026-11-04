// Thread-group-shared memory used with autodiff in the ways that stay correct. A direct read in a
// differentiable function saves its value for the backward pass, and an argument passed by value
// is expected to be a copy. A write of a whole variable and an atomic both stay in place, and so
// does a partial write in a non-differentiable function, which autodiff does not rewrite. Autodiff
// passes a reference argument through to the callee, so shared memory passed by reference under
// `no_diff`, whole or in part, is accessed in place by the callee. A `__constref` argument may be a
// copy whether or not autodiff is involved. A function with custom derivatives for both modes is
// called by autodiff rather than rewritten, so its primal and derivative bodies may write shared
// memory and synchronize, as long as the derivative does not depend on the primal's side effects.
// The cases that are reported are in writes.slang.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute -experimental-feature

RWStructuredBuffer<float> outputBuffer;

groupshared float g[4];
groupshared float gGrad[4];
groupshared float gScalar;
groupshared uint counter;

float readBorrowed(__constref float a)
{
    return a;
}

[Differentiable]
float byValue(no_diff float a, float x)
{
    return a * x;
}

[Differentiable]
float readShared(uint i, float x)
{
    return g[i] * x;
}

[Differentiable]
float sharedByValue(uint i, float x)
{
    return byValue(g[i], x);
}

[Differentiable]
float elementToBorrow(uint i, float x)
{
    return no_diff(readBorrowed(g[i])) * x;
}

float bump(__ref float a)
{
    a += 1.0;
    return a;
}

float sumAll(__ref float a[4])
{
    return a[0] + a[1] + a[2] + a[3];
}

[Differentiable]
float elementUnderNoDiff(uint i, float x)
{
    return x * no_diff(bump(g[i]));
}

[Differentiable]
float wholeArrayUnderNoDiff(float x)
{
    return x * no_diff(sumAll(g));
}

[Differentiable]
float wholeVariableWrite(float x)
{
    gScalar = 2.0;
    return x;
}

[Differentiable]
float atomicOnShared(float x)
{
    InterlockedAdd(counter, 1u);
    return x;
}

void writeElement(uint i, float v)
{
    g[i] = v;
}

[Differentiable]
float writeInNonDifferentiableFunction(uint i, float x)
{
    no_diff(writeElement(i, 1.0));
    return x;
}

float outsideDifferentiableCode(uint i, float x)
{
    g[i] = x;
    return g[i] * x;
}

[Differentiable]
[ForwardDerivative(customForBothModesFwd)]
[BackwardDerivative(customForBothModesBwd)]
float customForBothModes(uint i, float x)
{
    return g[i] * x;
}

DifferentialPair<float> customForBothModesFwd(uint i, DifferentialPair<float> x)
{
    return diffPair(g[i] * x.p, g[i] * x.d);
}

void customForBothModesBwd(uint i, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, g[i] * dOut);
}

[Differentiable]
float customByDerivativeOf(uint i, float x)
{
    return g[i] * x;
}

[ForwardDerivativeOf(customByDerivativeOf)]
DifferentialPair<float> customByDerivativeOfFwd(uint i, DifferentialPair<float> x)
{
    return diffPair(g[i] * x.p, g[i] * x.d);
}

[BackwardDerivativeOf(customByDerivativeOf)]
void customByDerivativeOfBwd(uint i, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, g[i] * dOut);
}

[BackwardDerivative(exchangeWithNeighborBwd)]
float exchangeWithNeighbor(uint i, float x)
{
    g[i] = float(i) * 10.0;
    GroupMemoryBarrierWithGroupSync();
    return g[(i + 1) % 4] * x;
}

// The backward pass may skip the primal call when it does not need the result, so the derivative
// recomputes the shared values instead of relying on the primal's writes.
void exchangeWithNeighborBwd(uint i, inout DifferentialPair<float> x, float dOut)
{
    gGrad[i] = float(i) * 10.0;
    GroupMemoryBarrierWithGroupSync();
    x = diffPair(x.p, gGrad[(i + 1) % 4] * dOut);
}

[Differentiable]
float throughCustomDerivative(uint i, float x)
{
    return exchangeWithNeighbor(i, x) * 2.0;
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    var y = diffPair(3.0);
    bwd_diff(throughCustomDerivative)(i, y, 1.0);
    outputBuffer[i] = y.d + readShared(i, 1.0) + sharedByValue(i, 1.0) +
                      elementUnderNoDiff(i, 1.0) + wholeArrayUnderNoDiff(1.0) +
                      elementToBorrow(i, 1.0) + wholeVariableWrite(1.0) + atomicOnShared(1.0) +
                      writeInNonDifferentiableFunction(i, 1.0) +
                      outsideDifferentiableCode(i, 1.0) + customForBothModes(i, 1.0) +
                      customByDerivativeOf(i, 1.0);
}
