// A write through a `groupshared` parameter reaches the caller's `__shared__` storage, not a
// per-thread copy (shader-slang/slang#10641). The PTX run compiles the emitted CUDA through NVRTC,
// which rejects member access on a pointer, the failure when the by-reference parameter is
// mistaken for a by-value array.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute -capability cuda_sm_7_0

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

// CUDA: __shared__ {{.*}}groupSharedData
// CUDA: groupSharedData{{.*}}] =

// PTX: computeMain
