// Positive companion to the #12367 diagnostic tests: a `functype` parameter whose argument
// names a function directly IS supported on kernel targets, and must keep compiling.
//
// `isParamSuitableForSpecialization` accepts an `IRGlobalValueWithCode` argument, so
// `specializeHigherOrderParameters` replaces the call with a direct one and no function type
// ever reaches emission. This is the shape users should write, and it is what E55215 points
// them toward -- so this test guards against that diagnostic being too broad. If the check
// starts keying on the function *type* rather than on a function-typed value being stored,
// every one of these targets would begin erroring on correct code.
//
// `CHECK-NOT` is meaningful here: these are `TEST:SIMPLE(filecheck=...)` cases, which run
// real LLVM FileCheck over the emitted text. (In a `DIAGNOSTIC_TEST`, `CHECK-NOT` would be
// inert -- that harness only matches lines beginning exactly `//CHECK:`.)

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

//CUDA: computeMain
//CUDA-NOT: Slang_FuncType

//CPP: computeMain
//CPP-NOT: Slang_FuncType

//METAL: computeMain
//METAL-NOT: Func<

//WGSL: computeMain
//WGSL-NOT: {{: *;}}
