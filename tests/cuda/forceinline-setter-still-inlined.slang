// A user `[ForceInline]` on a property setter must still be inlined by Slang on CUDA, NOT deferred
// to NVRTC. The compiler force-inlines setters for first-`inout`-parameter initialization
// correctness; that requirement co-exists with the user hint here, so the CUDA deferral must not
// fire. See shader-slang/slang#12623.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute

// The setter is inlined into the kernel; no separate `__forceinline__ __device__` setter survives.
// CUDA-NOT: __forceinline__

RWStructuredBuffer<int> outputBuffer;

struct Wrapper
{
    int stored;

    property int value
    {
        [ForceInline]
        set { stored = newValue; }
        get { return stored; }
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Wrapper w;
    w.value = int(tid.x);
    outputBuffer[tid.x] = w.value;
}
