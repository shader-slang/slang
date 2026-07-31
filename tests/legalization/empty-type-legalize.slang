// Regression for shader-slang/slang#11917: `legalizeEmptyTypes` has an entry-time early-out that
// skips the pass when its presence scan (`hasEmptyTypeLegalizationWork` via
// `isEmptyTypeShapeToLegalize`) finds no empty-type-shape root. This module has an empty `struct` the
// pass must remove (C/C++ backends trip over zero-size aggregates), used both directly (`useEmpty`)
// and as an array element (`useEmptyArray`). The checks assert the pass ran: each empty parameter is
// legalized away so the emitted signature keeps only its `int`, and no `struct Empty` survives
// anywhere. If the scan wrongly skipped, the parameters and their `struct Empty` would reach emit.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain

struct Empty
{
};

// Return a value so the empty struct participates in a real signature and survives to legalization
// rather than being dead-code-eliminated before the pass runs.
int useEmpty(Empty e, int x)
{
    return x;
}

int useEmptyArray(Empty arr[3], int x)
{
    return x;
}

RWStructuredBuffer<int> outputBuffer;

// `struct Empty` must not survive anywhere in the emitted C++. A `CHECK-NOT` is only scanned in the
// gap between its surrounding positive matches, so to cover the whole file the negative is repeated
// in every gap: before the first positive (`#include`, the top of output), between the positives,
// and after the last (which extends to end-of-file). The positives also assert each empty parameter
// was legalized away — only the `int` survives in `useEmpty` / `useEmptyArray`.
// CHECK-NOT: struct Empty
// CHECK: #include
// CHECK-NOT: struct Empty
// CHECK: useEmpty{{.*}}(int{{.*}} {{[A-Za-z0-9_]+}})
// CHECK-NOT: struct Empty
// CHECK: useEmptyArray{{.*}}(int{{.*}} {{[A-Za-z0-9_]+}})
// CHECK-NOT: struct Empty
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Empty e = {};
    Empty arr[3];
    outputBuffer[tid.x] = useEmpty(e, int(tid.x)) + useEmptyArray(arr, int(tid.x));
}
