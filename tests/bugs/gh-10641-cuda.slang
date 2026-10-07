// Regression test for the CUDA and Metal handling of a bare `groupshared` array parameter
// (https://github.com/shader-slang/slang/issues/10641).
//
// A bare `groupshared` array parameter is passed by reference, so a write through it must land on
// the caller's group-shared storage and never on a per-thread copy. Inlining the callee is the
// default legalization and `[noinline]` does not prevent it, so what these lanes pin is the storage
// the write reaches once the callee is gone: `__shared__` on CUDA, `threadgroup` on Metal. The
// `-target ptx` lane additionally compiles the emitted CUDA through NVRTC, which rejects member
// access on a pointer -- the failure mode when the by-reference parameter is mistaken for a
// by-value array.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute -capability cuda_sm_7_0
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const uint N = 8;
groupshared uint groupSharedData[N];

// The array itself is the group-shared storage, and the write reaches it directly rather than a
// local copy. A per-thread copy -- the #10641 defect -- would appear as a plain (non-`__shared__`)
// declaration written instead.
// CUDA: __shared__ {{.*}}groupSharedData
// CUDA: groupSharedData{{.*}}] =
//
// Metal spells the same storage `threadgroup`.
// METAL: threadgroup {{.*}}groupSharedData
// METAL: groupSharedData{{.*}}] =
//
// NVRTC accepts the emitted CUDA and produces PTX for the entry point.
// PTX: computeMain
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
