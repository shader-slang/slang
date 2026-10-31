// Regression test for shader-slang/slang#12785 (negative: intervening write / barrier).
//
// Dominance alone is not sufficient to reuse a read-only-resource-load call: the Slang memory model
// ("Memory Aliasing via Binding") permits a `StructuredBuffer` (SRV) and an `RWStructuredBuffer`
// (UAV) bound to the same memory to alias, and a `GroupMemoryBarrierWithGroupSync` establishes an
// ordering. When a memory-writing or synchronizing instruction lies between two identical calls,
// the second call must NOT be commoned -- it could observe a changed value.
//
// Three entry points, each asserting both calls survive: an intervening UAV store, an intervening
// barrier, and an intervening atomic (an opaque side-effecting op, exercising the between-walk's
// conservative default).

//TEST:SIMPLE(filecheck=CHECK_WRITE): -target cuda -entry mainWithWrite -stage compute
//TEST:SIMPLE(filecheck=CHECK_BARRIER): -target cuda -entry mainWithBarrier -stage compute
//TEST:SIMPLE(filecheck=CHECK_ATOMIC): -target cuda -entry mainWithAtomic -stage compute

StructuredBuffer<float> inputData;
RWStructuredBuffer<float> outputData;
RWStructuredBuffer<uint> counter;

[noinline]
float loadValue(uint index)
{
    return inputData[index];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void mainWithWrite(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    float a = loadValue(index);
    outputData[index] = 42.0f;       // may alias inputData; must block the CSE
    float b = loadValue(index);
    outputData[index] = a + b;
}

// CHECK_WRITE-COUNT-2: loadValue_0(index

[shader("compute")]
[numthreads(4, 1, 1)]
void mainWithBarrier(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    float a = loadValue(index);
    GroupMemoryBarrierWithGroupSync();   // synchronization; must block the CSE
    float b = loadValue(index);
    outputData[index] = a + b;
}

// CHECK_BARRIER-COUNT-2: loadValue_0(index

[shader("compute")]
[numthreads(1, 1, 1)]
void mainWithAtomic(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    float a = loadValue(index);
    uint old;
    InterlockedAdd(counter[0], 1u, old);   // opaque side-effecting op; must block the CSE
    float b = loadValue(index);
    outputData[index] = a + b;
}

// CHECK_ATOMIC-COUNT-2: loadValue_0(index
