// A new-mode parameter reached through a dynamic-dispatch (interface) call site
// must flow through typeflow specialization to the same code-generation-unsupported
// diagnostic, rather than aborting with an internal "unhandled parameter direction"
// error inside the typeflow pass.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -stage compute

interface IFoo
{
    int compute(int v);
}

struct Impl : IFoo
{
    int compute(int v) { return v + 1; }
}

int useDyn(__ref_readonly int x)
{
/*CHECK: ([[# @LINE-2]]): error 56006
*/
    return x + 1;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Impl impl;
    IFoo foo = impl;
    int v = foo.compute(3);
    outputBuffer[0] = useDyn(v);
}
