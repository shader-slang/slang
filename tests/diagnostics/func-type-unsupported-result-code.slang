// Companion to the other #12367 tests, pinning the compiler's *result code* rather than its
// diagnostic text.
//
// The `DIAGNOSTIC_TEST` harness extracts only the `standard error` block from the run's output
// (`_diagnosticAnnotationTest` in slang-test-main.cpp) and never looks at the result code, so those
// tests would still pass if the compiler reported E55216 and then crashed. That distinction matters
// here: on `llvm-shader-ir` this shape used to be a segmentation fault, and the point of rejecting
// it is that compilation now ends in an ordinary error. A plain `.expected` comparison records the
// result code, so it is what pins that.

//TEST:SIMPLE: -target llvm-shader-ir -entry computeMain -stage compute

int addOne(int x)
{
    return x + 1;
}

static functype(int) -> int gFn = addOne;

int applyIt(functype(int) -> int f, int x)
{
    return f(x);
}

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = applyIt(gFn, 41);
}
