// A write through a `groupshared` parameter reaches the caller's `threadgroup` storage, not a
// per-thread copy (shader-slang/slang#10641). The execution run is in
// tests/language-feature/groupshared-param/by-reference.slang.

//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute

RWStructuredBuffer<uint> outputBuffer;

static const uint N = 8;
groupshared uint groupSharedData[N];

[noinline]
void writeSlot(uint tid, groupshared uint scratch[N])
{
    scratch[tid] = tid + 1;
}

[shader("compute")]
[numthreads(N, 1, 1)]
void computeMain(uint3 tid : SV_GroupThreadID)
{
    writeSlot(tid.x, groupSharedData);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[tid.x] = groupSharedData[(tid.x + 1) % N];
}

// METAL: threadgroup {{.*}}groupSharedData
// METAL: groupSharedData{{.*}}] =
