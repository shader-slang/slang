//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -stage compute -entry computeMain -target cuda

// The generic `dot` fallback force-unrolls its accumulation loop, but a forced unroll that
// exceeds the unroller's iteration cap is a hard error and `validateVectorsAndMatrices` runs
// after unrolling. An out-of-range width must therefore stay out of the unrolled loop, or the
// reported error is the unroll failure rather than the invalid width the user needs to see.

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    vector<float, 5000> a = (vector<float, 5000>)1.0f;
    outputBuffer[0] = dot(a, a);
//CHECK:                 ^ see call to 'dot'
}

// What this pins is the `static_assert` on the width: without it only the element-count diagnostic
// is reported, and because `diag=` matching is exhaustive the unannotated-vs-annotated mismatch
// fails the test. It does not pin the order in which the unroller and the width validator run —
// the assert rejects the width before the unroller is reached, so no unroll failure arises here to
// assert about. That ordering issue is tracked in shader-slang/slang#12420.
//
// Because the matching is exhaustive, an unexpected diagnostic also fails the test for lack of an
// annotation. Do not add a `CHECK-NOT` for that — it is a FileCheck directive and is inert here.
//CHECK: static assertion failed
//CHECK: static assertion failed, dot is only implemented for vectors of up to 4 elements
//CHECK: invalid vector element count
//CHECK: vector has invalid element count '5000', valid values are between '1' and '4' inclusive
