// Thread-group-shared memory can reach a differentiated function's writable reference through an
// unqualified `__ref` parameter, which accepts storage in any address space. Consider `h` passing
// its shared parameter to `g`, which passes it on to `bwd_diff(f)`. Inside `g`, nothing says that
// `a` is shared memory, so the `bwd_diff` call cannot be checked locally. The call in `h` is the last
// place where the argument is visibly shared, so it should warn there.
//
// Disabled: catching this needs either an address space in the reference parameter's signature,
// so that the call in `h` is diagnosed as a mismatch, or an analysis across functions. Neither
// exists today. The expected diagnostic and its location depend on which approach is chosen.

//DISABLE_DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute -experimental-feature

RWStructuredBuffer<float> outputBuffer;
groupshared float gs[4];

[Differentiable]
float f(no_diff __ref float a, float x)
{
    a = a + 1;
    return a * x;
}

float g(__ref float a)
{
    var x = diffPair(2.0);
    bwd_diff(f)(a, x, 1.0);
    return x.d;
}

float h(groupshared float a)
{
    return g(a);
//CHECK:     ^ passing thread-group-shared memory by reference introduces a race condition when this function is differentiated
//CHECK:     ^ pass it to a '__ref_readonly' parameter, or access it directly as a global in the callee
}

[numthreads(4, 1, 1)]
void computeMain(uint i : SV_GroupIndex)
{
    outputBuffer[i] = h(gs[i]);
}
