//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage compute -entry computeMain

// Regression test for shader-slang/slang#12563 (found while reviewing the fix
// for shader-slang/slang#12498).
//
// `pick` returns pointers of two different address spaces on its two paths: the
// caller's local `fp` ('Function' storage) and the `groupshared` global
// `gShared` ('Workgroup' storage). A function has a single result type, so this
// is target-invalid — but the address-space specialization pass must still
// terminate. It derives the function's result address space from the first
// return in iteration order (here `fp`, Function) rather than letting the last
// return processed win; otherwise the two returns would flip the result type on
// every fixpoint drain and requeue the function forever. With a deterministic
// choice the pass converges and reports E58004 cleanly instead of hanging.

RWStructuredBuffer<int> outputBuffer;

groupshared int gShared;

[noinline]
int* pick(int* fp, bool c)
/*CHECK:
     ^^^^ E58004
     ^^^^ a function-local ('Function')
*/
{
    if (c)
        return fp;
    return &gShared;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    int local = int(tid.x);
    gShared = 1;
    int* r = pick(&local, tid.x == 0);
    outputBuffer[0] = *r;
}
