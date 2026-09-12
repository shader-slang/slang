//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute

// Regression test for shader-slang/slang#12498.
//
// SPIR-V's Logical addressing model only permits a function to return a pointer
// in the Workgroup, StorageBuffer, or PhysicalStorageBuffer storage class (a
// fixed whitelist, not a lifetime rule). Returning a pointer to a function-local
// ('Function' storage) or to a 'static' global ('Private' storage) is
// unrepresentable and must be diagnosed rather than emitted as invalid SPIR-V. Both helpers are
// `[noinline]` so the pointer is genuinely returned across a call boundary, and
// both arms return the same `p` deliberately so it is returned along more than
// one control-flow path. One entry point calls both so both diagnostics fire in
// a single compile.

RWStructuredBuffer<int> outputBuffer;

// 'Function' storage class: a pointer to the caller's local `v`.
[noinline]
int* pickLocal(int* p, bool cond)
/*CHECK:
     ^^^^^^^^^ E58004
     ^^^^^^^^^ a function-local ('Function')
*/
{
    if (cond)
        return p;
    return p;
}

// 'Private' storage class: a pointer to a `static` global. Also unreturnable in
// Logical addressing, so the same E58004 must fire.
static int gStatic;

[noinline]
int* pickStatic(int* p, bool cond)
/*CHECK:
     ^^^^^^^^^^ E58004
     ^^^^^^^^^^ a 'static' global ('Private')
*/
{
    if (cond)
        return p;
    return p;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    int v = int(tid.x);
    int* a = pickLocal(&v, tid.x > 0);
    gStatic = int(tid.x);
    int* b = pickStatic(&gStatic, tid.x > 0);
    outputBuffer[0] = *a + *b;
}
