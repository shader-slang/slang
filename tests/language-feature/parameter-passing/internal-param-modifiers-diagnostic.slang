// The internal `__ref_readonly` / `__ref_writeonly` / `__consume` parameter modes
// are representable but have no code-generation support yet. A function using one
// that is reachable from an entry point (so it survives inlining and dead-code
// elimination to reach code generation) is diagnosed uniformly for every target,
// rather than silently emitting a plain by-value parameter that would drop the
// mode's by-reference / ownership contract. The diagnostic is raised by a single
// target-independent pass, so it also fires on the host-VM and
// minimum-optimization paths that skip the general unsupported-inst check.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target slang-vm -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -stage compute -minimum-slang-optimization

RWStructuredBuffer<int> outputBuffer;

int useRefReadOnly(__ref_readonly int x)
{
/*CHECK: ([[# @LINE-1]]): error 56006
*/
    return x + 1;
}

int useRefWriteOnly(__ref_writeonly int x)
{
/*CHECK: ([[# @LINE-1]]): error 56006
*/
    x = 5;
    return 100;
}

int useConsume(__consume int x)
{
/*CHECK: ([[# @LINE-1]]): error 56006
*/
    return x * 2;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    int a = 1;
    outputBuffer[0] = useRefReadOnly(a) + useRefWriteOnly(a) + useConsume(a);
}
