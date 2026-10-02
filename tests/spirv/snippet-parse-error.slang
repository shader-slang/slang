//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

// An unrecognized SPIR-V opcode in a `__target_intrinsic(spirv, ...)` snippet must be reported as
// snippet-parsing-failed (E29000), not crash the compiler. Two calls to the same malformed
// intrinsic must still yield exactly one E29000 — the parse failure is cached and diagnosed once,
// and exhaustive diagnostic matching fails on any extra diagnostic.

RWStructuredBuffer<float> outBuf;

//CHECK: E29000
//CHECK: snippet
__target_intrinsic(spirv, "OpTotallyBogusOpcode resultType resultId _0 _1")
float badSnippet(float a, float b);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = badSnippet(1.0f, 2.0f) + badSnippet(3.0f, 4.0f);
}
