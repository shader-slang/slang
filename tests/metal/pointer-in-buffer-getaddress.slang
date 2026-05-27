//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -stage compute -entry computeMain

// Verify that __getAddress on a buffer element is correctly rejected on Metal.
// __getAddress(ptrs[0]) would produce int** (Ptr<int*>), but Metal doesn't
// allow taking the address of buffer elements this way. The front-end should
// emit an appropriate diagnostic.
//
// This addresses reviewer feedback about the __addressOf edge case.
//
// See: https://github.com/shader-slang/slang/issues/10841

RWStructuredBuffer<int*> ptrs;
RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int** pp = __getAddress(ptrs[0]);
//CHECK:       ^^^^^^^^^^^^ invalid __getAddress usage
//CHECK:       ^^^^^^^^^^^^ '__getAddress' cannot take the address of a function-local variable on this target.
    output[tid.x] = **pp;
}
