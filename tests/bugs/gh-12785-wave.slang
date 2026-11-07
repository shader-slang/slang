// Regression test for shader-slang/slang#12785 (negative: execution-mask-dependent callee).
//
// A `[noinline]` wrapper containing a wave/quad/derivative op is NOT side-effect-free: its value
// depends on the set of active lanes, which changes across a divergent branch. Such a wrapper must
// never be commoned by the resource-load-call CSE, even when the calls are structurally identical
// and the first dominates the second. This is guaranteed automatically: the eligibility walk
// (`calleeIsRepeatableReadOnly`) sees `WaveActiveSum` report a side effect via
// `mightHaveSideEffects()` and rejects the wrapper directly -- the gate never consults a
// `NoSideEffect` property. Here the dominated call sits in a divergent branch, exactly the case that
// would be miscompiled if a mask-dependent value were reused from the wider-mask dominating call.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute

StructuredBuffer<float> inputData;
RWStructuredBuffer<float> outputData;

[noinline]
float waveWrap(uint index)
{
    return WaveActiveSum(inputData[index]);
}

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    float first = waveWrap(index);
    float acc = first;
    if ((index & 1) != 0)
    {
        acc += waveWrap(index);
    }
    outputData[index] = acc;
}

// Both calls must survive (mask-dependent value must not be reused across the divergent branch).
// CHECK-COUNT-2: waveWrap_0(index
