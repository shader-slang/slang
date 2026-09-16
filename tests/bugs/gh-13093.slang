// Regression test for issue #13093: `countof` on an unsized array must produce a clean
// semantic-check diagnostic (E56002), not an internal error (E99999) during code emit.
//
// A truly unsized array (`int gArr[];`) has no compile-time element count, so `countof`
// on it cannot be folded and previously survived to code emit as a `kIROp_CountOf` that
// no backend handles. The checker now rejects it up front, matching the diagnostic that
// `arr.getCount()` on an unsized array already produces (see tests/bugs/gh-6698.slang).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target cpp -entry computeMain -stage compute

RWStructuredBuffer<int> outBuf;

int gArr[];

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = countof(gArr);
    //CHECK: E56002
    //CHECK: cannot obtain the size of an unsized array
}
