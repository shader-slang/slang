//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage compute -entry computeMain -disable-non-essential-validations

// Regression test for shader-slang/slang#12563 (found while reviewing the fix
// for shader-slang/slang#12498).
//
// A directly recursive, pointer-returning function must not crash the
// address-space specialization pass. `-disable-non-essential-validations` turns
// off the recursion check (E55201) that would normally reject `recurse`, so the
// pass sees a cyclic call graph. Specializing a recursive function used to clone
// the specialized clone again on every recursive self-call — the clone's
// self-call targets the clone, whose parameters are not yet marked
// already-specialized — recursing without bound until the stack overflowed
// (SIGSEGV). Specialization now stops once a callee already has the requested
// parameter address spaces, so the pass terminates and reports E58004 (the
// returned pointer is in the 'Function' storage class, which SPIR-V's Logical
// addressing model cannot return) instead of crashing.

RWStructuredBuffer<int> outputBuffer;

[noinline]
int* recurse(int* p, int n)
/*CHECK:
     ^^^^^^^ E58004
     ^^^^^^^ PhysicalStorageBuffer
*/
{
    if (n <= 0)
        return p;
    return recurse(p, n - 1);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    int v = 0;
    int* r = recurse(&v, 4);
    outputBuffer[0] = *r;
}
