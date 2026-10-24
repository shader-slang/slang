// Autodiff passes a reference argument by copy-in/copy-out, so a callee given thread-group-shared
// memory through a reference works on a private copy of it. We warn where the shared memory
// enters the chain of reference parameters: a call inside a differentiable function, including
// one under `no_diff`, or a call to `fwd_diff(f)` or `bwd_diff(f)`. A differentiable function
// that forwards its own reference parameter is not warned, and neither is a call outside
// differentiable code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;
groupshared float g[4];

[Differentiable]
float helper(no_diff groupshared float a, float x)
{
    return a * x;
}

// Not warned: forwards its own parameter.
[Differentiable]
float mid(no_diff groupshared float a, float x)
{
    return helper(a, x);
}

[Differentiable]
float f(float x, uint j)
{
    float r = mid(g[j], x);
//CHECK:           ^ argument names thread-group-shared memory, which other invocations can access during the call, but a differentiable function passes a reference argument to 'a' by copy-in/copy-out
//CHECK:           ^ the callee works on a private copy, so it misses other invocations' writes during the call, and copying back can overwrite them
    return r + no_diff(helper(g[0], x));
//CHECK:                       ^ argument names thread-group-shared memory, which other invocations can access during the call, but a differentiable function passes a reference argument to 'a' by copy-in/copy-out
//CHECK:                       ^ the callee works on a private copy, so it misses other invocations' writes during the call, and copying back can overwrite them
}

// Not warned: outside differentiable code.
float plain(uint j, float x)
{
    return helper(g[j], x);
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    var x = diffPair(3.0);
    bwd_diff(mid)(g[i], x, 1.0);
//CHECK:           ^ argument names thread-group-shared memory, which other invocations can access during the call, but a differentiable function passes a reference argument to 'a' by copy-in/copy-out
//CHECK:           ^ the callee works on a private copy, so it misses other invocations' writes during the call, and copying back can overwrite them
    float d = fwd_diff(helper)(g[i], diffPair(1.0, 1.0)).d;
//CHECK:                        ^ argument names thread-group-shared memory, which other invocations can access during the call, but a differentiable function passes a reference argument to 'a' by copy-in/copy-out
//CHECK:                        ^ the callee works on a private copy, so it misses other invocations' writes during the call, and copying back can overwrite them
    outputBuffer[i] = x.d + d + plain(i, 1.0) + f(1.0, i);
}
