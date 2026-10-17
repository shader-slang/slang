//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage compute -entry computeMain

// Regression test for shader-slang/slang#12563 (found while reviewing the fix
// for shader-slang/slang#12498).
//
// `pick` returns pointers of two different address spaces on its two paths: the
// caller's local `fp` ('Function' storage) and the `groupshared` global `gShared`
// ('Workgroup'). A function has one result type, so this is target-invalid, but
// the pass must still terminate: it derives the result address space from the
// first return in iteration order (here `fp`) rather than letting the last win,
// so the two returns can't flip the result type every drain and requeue forever.
//
// Two distinct errors fire: E58005 (the returns disagree, from the shared
// address-space pass) and E58004 (the chosen result class 'Function' is not
// returnable in SPIR-V at all). The whitelist-internal case where only E58005
// applies (both returns in different but individually returnable classes) is
// conflicting-returnable-addrspace-12563.slang.

RWStructuredBuffer<int> outputBuffer;

groupshared int gShared;

[noinline]
int* pick(int* fp, bool c)
/*CHECK:
     ^^^^ E58004
     ^^^^ PhysicalStorageBuffer
*/
{
    if (c)
        return fp;
    return &gShared;
    /*CHECK:
    ^^^^^^ E58005
    ^^^^^^ single address space
    */
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    int local = int(tid.x);
    gShared = 1;
    int* r = pick(&local, tid.x == 0);
    outputBuffer[0] = *r;
}
