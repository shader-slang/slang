//TEST:COMPILE: tests/autodiff/diff-library-module.slang -o tests/autodiff/diff-library-module.slang-module
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

// Regression test for the prelink-vs-final-link distinction behind
// `IRSharedSpecContext::isFinalCodegenLink` (slang-ir-link.cpp): only the
// final per-target codegen link may prune auto-diff link artifacts; the
// prelink that produced the serialized diff-library-module.slang-module
// (import prefers the precompiled module over source) must keep annotations
// and witness-table entries complete, because this consumer differentiates a
// library function even though the library itself never does. If prelink ever
// prunes — e.g. a refactor sets `isFinalCodegenLink` in `prelinkIR` — the
// derivative of `libSquare` is unavailable here and this test fails.

import diff_library_module;

typedef DifferentialPair<float> dpfloat;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // d/dx (x*x) at x = 5 is 10.
    outputBuffer[0] = __fwd_diff(libSquare)(dpfloat(5.0, 1.0)).d;
    // CHECK: 10.0
}
