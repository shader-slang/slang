// The property accessor the compiler synthesizes to satisfy an interface requirement carries a
// compiler-generated `[ForceInline]`. That is a compiler-mandated inline, not a user hint, so it
// must still be inlined by Slang on CUDA and not deferred to NVRTC — even though it is a `get`
// accessor with no user-authored attribute (shader-slang/slang#12623).

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute

// The synthesized accessor is inlined; no separate `__forceinline__ __device__` accessor survives.
// CUDA-NOT: __forceinline__

interface IHasVal
{
    property int val { get; }
}

struct Thing : IHasVal
{
    int val; // satisfies the property; the compiler synthesizes the [ForceInline] getter
}

int useIt<T : IHasVal>(T t)
{
    return t.val;
}

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Thing t;
    t.val = int(tid.x);
    outputBuffer[tid.x] = useIt(t);
}
