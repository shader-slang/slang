// Regression test for shader-slang/slang#12785 (a write on one arm of a diamond, between the
// dominating and dominated calls, blocks reuse across the merge — with an empty-arm control).
//
// `a` (before the branch) dominates `b` (after the merge), so `b` is a reuse candidate. The
// between-walk (`noMemoryClobberBetween`) collects every block between the two calls across the
// diamond and rejects reuse on ANY side-effecting instruction (it does no per-address alias
// analysis, since distinct bindings may alias the same storage). `mainStore` puts a store on one
// arm, so the reuse is blocked and both calls survive; `mainEmpty` has the identical diamond shape
// but its branch writes only a local, so nothing side-effecting lies between and the two calls are
// commoned. The contrast confirms it is the store, not the branch shape, that blocks — and both
// exercise the multi-block predecessor collection on the negative and positive sides respectively.

//TEST:SIMPLE(filecheck=STORE): -target cuda -entry mainStore -stage compute
//TEST:SIMPLE(filecheck=EMPTY): -target cuda -entry mainEmpty -stage compute

StructuredBuffer<uint> inputData;
RWStructuredBuffer<uint> outputData;

[noinline]
uint loadValue(uint index)
{
    return inputData[index];
}

// Store on one arm, between the two calls: reuse across the merge is blocked.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainStore(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    uint a = loadValue(index);
    if ((index & 1) != 0)
    {
        outputData[index] = 7;
    }
    uint b = loadValue(index);
    outputData[index] = a + b;
}
// STORE-COUNT-2: loadValue_0(index
// STORE-NOT: loadValue_0(index

// Identical diamond, but the arm writes only a local: nothing side-effecting between, so commoned.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainEmpty(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    uint a = loadValue(index);
    uint g = 0;
    if ((index & 1) != 0)
    {
        g = 1;
    }
    uint b = loadValue(index);
    outputData[index] = a + b + g;
}
// EMPTY-COUNT-1: loadValue_0(index
// EMPTY-NOT: loadValue_0(index
