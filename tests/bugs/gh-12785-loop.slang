// Regression test for shader-slang/slang#12785 (loop / back-edge in the between-walk).
//
// A dominating call in the loop preheader and an identical dominated call in the loop body exercise
// `noMemoryClobberBetween`'s back-edge handling: because the body is reachable from itself, the walk
// marks `userIsOwnPredecessor` and rescans the WHOLE body (not just up to the dominated call), so a
// clobber that runs AFTER the dominated call -- reached again on the next iteration before the call
// re-executes -- is still seen.
//
// A bare read-only call is not a clobber (it is `NoSideEffect`, so `mightHaveSideEffects()` is
// false), so with nothing writing memory the body call reuses the preheader call across iterations
// (a sound loop-invariant read-only reuse) -- `mainLoopNoStore` collapses to one call. But a store
// in the body IS a clobber, and the back-edge rescan catches it even though it sits after the
// dominated call -- `mainLoopStore` keeps both calls. Without the `userIsOwnPredecessor` rescan the
// store (after the call) would be missed and the two calls wrongly commoned across the store.

//TEST:SIMPLE(filecheck=STORE): -target hlsl -entry mainLoopStore -stage compute
//TEST:SIMPLE(filecheck=NOSTORE): -target hlsl -entry mainLoopNoStore -stage compute

StructuredBuffer<uint> inputData;
RWStructuredBuffer<uint> outputData;

[noinline]
uint loadValue(uint index)
{
    return inputData[index];
}

// Store in the body (on the back-edge path) blocks loop-carried reuse: both calls survive.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainLoopStore(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    uint a = loadValue(i);
    uint acc = a;
    for (uint k = 0; k < i; k++)
    {
        uint b = loadValue(i);
        outputData[i] = b + k; // clobber after the dominated call, caught by the back-edge rescan
        acc += b;
    }
    outputData[i] = acc;
}
// STORE-COUNT-2: loadValue_0(i
// STORE-NOT: loadValue_0(i

// No store between the two calls: the body call reuses the preheader call across iterations.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainLoopNoStore(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    uint a = loadValue(i);
    uint acc = a;
    for (uint k = 0; k < i; k++)
    {
        uint b = loadValue(i);
        acc += b + k;
    }
    outputData[i] = acc;
}
// NOSTORE-COUNT-1: loadValue_0(i
// NOSTORE-NOT: loadValue_0(i
