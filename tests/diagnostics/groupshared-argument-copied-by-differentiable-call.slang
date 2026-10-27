// Autodiff prepares a function body for differentiation by replacing the addresses it computes
// with values, so a reference argument that names part of thread-group-shared memory, or the
// function's own `groupshared` parameter, is copied in before the call and stored back after it.
// We warn on such arguments inside a differentiated body, including under `no_diff`. A
// `fwd_diff`/`bwd_diff` call outside a differentiated body passes the address of an element
// unchanged, and neither an atomic nor a call outside differentiated code is copied.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;
groupshared float g[4];

[Differentiable]
float helper(no_diff groupshared float a, float x)
{
    return a * x;
}

[Differentiable]
float mid(no_diff groupshared float a, float x)
{
    return helper(a, x);
//CHECK:          ^ passing thread-group-shared memory by reference introduces a race condition when this function is differentiated
//CHECK:          ^ access the groupshared variable directly as a global instead of passing it as an argument; if forward or backward differentiation of this function is needed, provide a custom derivative
}

[Differentiable]
float f(float x, uint j)
{
    float r = mid(g[j], x);
//CHECK:           ^ passing thread-group-shared memory by reference introduces a race condition when this function is differentiated
//CHECK:           ^ access the groupshared variable directly as a global instead of passing it as an argument; if forward or backward differentiation of this function is needed, provide a custom derivative
    return r + no_diff(helper(g[0], x));
//CHECK:                       ^ passing thread-group-shared memory by reference introduces a race condition when this function is differentiated
//CHECK:                       ^ access the groupshared variable directly as a global instead of passing it as an argument; if forward or backward differentiation of this function is needed, provide a custom derivative
}

groupshared uint counter;

// Not warned: an atomic operates on the shared memory in place.
[Differentiable]
float countCalls(float x)
{
    InterlockedAdd(counter, 1u);
    return x;
}

// Not warned: outside differentiated code.
float plain(uint j, float x)
{
    return helper(g[j], x);
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    var x = diffPair(3.0);
    // Not warned: the element's address is passed unchanged.
    bwd_diff(mid)(g[i], x, 1.0);
    float d = fwd_diff(helper)(g[i], diffPair(1.0, 1.0)).d;
    outputBuffer[i] = x.d + d + plain(i, 1.0) + f(1.0, i) + countCalls(1.0);
}
