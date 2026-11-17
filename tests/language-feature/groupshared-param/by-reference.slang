// A `groupshared` parameter is a reference to the caller's thread-group-shared storage, so writes
// through it are visible to the other invocations in the group (shader-slang/slang#10641). Before,
// a bare `groupshared` array parameter was passed by value, and each invocation wrote its own copy.
//
// Each invocation writes its own slot through the parameter and, after a barrier, reads its
// neighbor's. Each invocation also increments a counter through a `groupshared` parameter with an
// atomic, so the counter ends at the group size only if every atomic reaches the same storage.

//TEST:SIMPLE(filecheck=HLSL): -entry computeMain -stage compute -target hlsl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-dx12 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-mtl -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const uint N = 8;
groupshared uint groupSharedData[N];
groupshared Atomic<uint> groupSharedCounter;

void shiftLeftByOne(uint tid, groupshared uint scratch[N])
{
    scratch[tid] = tid + 1;
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[tid] = scratch[(tid + 1) % N];
}

void count(groupshared Atomic<uint> counter)
{
    counter.add(1);
}

[shader("compute")]
[numthreads(N, 1, 1)]
void computeMain(uint3 tid : SV_GroupThreadID)
{
    if (tid.x == 0)
        groupSharedCounter.store(0);
    shiftLeftByOne(tid.x, groupSharedData);

    count(groupSharedCounter);
    GroupMemoryBarrierWithGroupSync();
    if (tid.x == 0)
        outputBuffer[N] = groupSharedCounter.load();
}

// The accesses land on the shared global, with no per-invocation copy of it between its
// declaration and the store, and no direction keyword on a `groupshared` parameter, which DXC
// rejects.
// HLSL: groupshared uint {{.*}}groupSharedData
// HLSL-NOT: [int({{[0-9]+}})] = {{.*}}scratch
// HLSL-NOT: uint {{[A-Za-z_0-9]+}}[int({{[0-9]+}})] = groupSharedData
// HLSL-NOT: groupshared inout
// HLSL-NOT: inout groupshared
// HLSL: groupSharedData{{.*}}] =
// HLSL: GroupMemoryBarrierWithGroupSync
// HLSL: outputBuffer{{.*}}] = groupSharedData

// Invocation i reads slot (i + 1) % N, which holds ((i + 1) % N) + 1, and the counter reaches N.
// BUF: type: uint32_t
// BUF-NEXT: 2
// BUF-NEXT: 3
// BUF-NEXT: 4
// BUF-NEXT: 5
// BUF-NEXT: 6
// BUF-NEXT: 7
// BUF-NEXT: 8
// BUF-NEXT: 1
// BUF-NEXT: 8
