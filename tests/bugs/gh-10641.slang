// Regression test for https://github.com/shader-slang/slang/issues/10641
//
// A bare `groupshared` array parameter (one with no explicit `in`/`out`/`__ref`/
// `__constref` direction modifier) names a single thread-group-shared storage
// location and must be passed *by reference*. Before the fix it lowered to a
// by-value `in` parameter, so each invocation got its own private copy of the
// array: the cross-thread read below saw stale data and, on D3D/DXIL, no
// thread-group-shared (`addrspace(3)`) storage was emitted at all -- scan/reduce
// kernels silently returned garbage.

// Structural lane (runs everywhere, including CI without a GPU). The bug is
// visible in the helper's emitted body: passing the array by value copies the
// `groupshared` parameter into a per-thread local array (`uint l[int(8)] =
// scratch;`) and operates on the copy, so the shared store never happens. The
// by-reference fix removes that copy. We also guard the emit-side requirement
// that a `groupshared` parameter never carry an `in`/`out`/`inout` direction
// keyword, which DXC rejects (e.g. `groupshared inout uint s[8]`).
//TEST:SIMPLE(filecheck=HLSL): -entry computeMain -stage compute -target hlsl

// Behavioral lanes on the targets where the bug manifests (run by CI, which has
// GPUs; skipped where no device is present).
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-dx12 -use-dxil -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const uint N = 8;
groupshared uint groupSharedData[N];

// `scratch` is a bare `groupshared` parameter: it aliases the caller's shared
// array. Each thread writes its own slot, then -- after a barrier -- reads its
// neighbour's slot, which only yields the right value if the array is genuinely
// shared across the group rather than copied per invocation.
// HLSL: void shiftLeftByOne
// HLSL-NOT: [int({{[0-9]+}})] = {{.*}}scratch
// HLSL-NOT: groupshared inout
// HLSL-NOT: inout groupshared
void shiftLeftByOne(uint tid, groupshared uint scratch[N])
{
    scratch[tid] = tid + 1;
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[tid] = scratch[(tid + 1) % N];
}

[shader("compute")]
[numthreads(N, 1, 1)]
void computeMain(uint3 tid : SV_GroupThreadID)
{
    shiftLeftByOne(tid.x, groupSharedData);
}

// Thread i reads slot (i+1)%N, holding ((i+1)%N)+1 -> [2,3,4,5,6,7,8,1]:
// BUF: 2
// BUF: 3
// BUF: 4
// BUF: 5
// BUF: 6
// BUF: 7
// BUF: 8
// BUF: 1
