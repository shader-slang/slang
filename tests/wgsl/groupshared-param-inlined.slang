// Regression coverage for the WGSL handling of a bare `groupshared` array parameter
// (https://github.com/shader-slang/slang/issues/10641).
//
// A bare `groupshared` array parameter aliases one thread-group-shared location and must not be
// copied per invocation. Baseline WGSL cannot take a `ptr<workgroup, ...>` as a function
// parameter, so the helper must be inlined away and the shared accesses must land directly on the
// `workgroup` global -- never on a `function`-space copy of the whole array that is written back
// wholesale (which would race and lose the sharing this issue is about).

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -output-using-type
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const uint N = 8;
groupshared uint groupSharedData[N];

// `scratch` is a bare `groupshared` parameter. On WGSL this helper must be inlined so the
// parameter disappears and the store lands directly on the `workgroup` global.
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

// The callee must be inlined: no `ptr<workgroup` parameter may survive.
// WGSL-NOT: ptr<workgroup
// The store must write the shared global directly, so a `var`-typed whole-array copy of it must
// not appear (that would be the copy-in/out temporary that loses sharing).
// WGSL-NOT: var {{[A-Za-z0-9_]+}} : array<u32,{{.*}}= groupSharedData
// WGSL: groupSharedData{{.*}}[{{.*}}] =

// Each thread i writes slot i (= i+1), then after the barrier reads slot (i+1)%N holding
// ((i+1)%N)+1, giving [2,3,4,5,6,7,8,1]:
// BUF: 2
// BUF: 3
// BUF: 4
// BUF: 5
// BUF: 6
// BUF: 7
// BUF: 8
// BUF: 1
