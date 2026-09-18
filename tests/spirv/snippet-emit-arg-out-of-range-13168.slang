//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

// #13168: a snippet argument reference `_N` is valid only for `N < argCount`. `outOfRangeArg` takes
// one argument (only `_0` is valid), so the `_1` reference is out of range and is diagnosed
// (E29001) during legalization, before emitSpvSnippet would index its argument array out of bounds.
// `_1` pins the `content >= argCount` boundary that an off-by-one `>` check would miss.

RWStructuredBuffer<float> outBuf;

//CHECK: E29001
//CHECK: references argument _1, but the call passes only 1 argument
//CHECK-NOT: E99997
__target_intrinsic(spirv, "OpNop _1")
float outOfRangeArg(float a);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = outOfRangeArg(1.0f);
}
