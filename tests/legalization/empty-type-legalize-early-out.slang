// Regression for shader-slang/slang#11917: `legalizeEmptyTypes` gained an entry-time early-out that
// skips the pass when the presence scan (`hasEmptyTypeLegalizationWork` via `isEmptyTypeToLegalize`)
// finds no empty type. This module has an empty `struct` passed to `useEmpty`, which the pass must
// remove (C/C++ backends trip over zero-size aggregates). The checks assert the pass ran: the empty
// parameter was legalized away so the emitted signature keeps only its `int`, and no `struct Empty`
// remains. If the scan wrongly skipped, both the parameter and its `struct Empty` would reach emit.

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

RWStructuredBuffer<int> outputBuffer;

// CHECK-NOT: struct Empty
// The empty-struct parameter was removed by legalization: only the `int` parameter survives.
// CHECK: useEmpty{{.*}}(int{{.*}} {{[A-Za-z0-9_]+}})
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Empty e = {};
    outputBuffer[tid.x] = useEmpty(e, int(tid.x));
}
