// Positive companion to the #12367 diagnostic tests: a `functype` parameter whose argument
// names a function directly IS supported on kernel targets, and must keep compiling.
//
// `isParamSuitableForSpecialization` accepts an `IRGlobalValueWithCode` argument, so
// `specializeHigherOrderParameters` replaces the call with a direct one and no function type
// ever reaches emission. This is the shape users should write, and it is what E55216 points
// them toward -- so this test guards against that diagnostic being too broad. If the check
// starts keying on the function *type* rather than on a function-typed value being stored,
// every one of these targets would begin erroring on correct code.
//
// Each `-NOT` is written BEFORE its positive anchor on purpose. FileCheck scopes a `-NOT` to
// the text between the preceding match and the next one, and every spelling below is emitted
// near the top of the file, ahead of the entry point -- placing the `-NOT` after a
// `computeMain` anchor would leave it scanning a region the spelling never appears in.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute

int addOne(int x)
{
    return x + 1;
}

int applyIt(functype(int) -> int f, int x)
{
    return f(x);
}

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = applyIt(addOne, 41);
}

// The function type must be specialized away rather than emitted. Each target has its own
// spelling for the undefined output it used to produce, and WGSL's failure mode is an empty
// type annotation with no type name at all, so each is excluded in the target's own terms.

//CUDA-NOT: Slang_FuncType
//CUDA: computeMain

//CPP-NOT: Slang_FuncType
//CPP: computeMain

//METAL-NOT: Func<
//METAL: computeMain

//WGSL-NOT: {{: *;}}
//WGSL: computeMain
