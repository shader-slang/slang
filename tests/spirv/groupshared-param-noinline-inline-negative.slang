// Contrast to groupshared-param-noinline.slang: an ORDINARY (force-inlined, no `[noinline]`)
// groupshared-parameter helper is inlined away on direct SPIR-V, so its Workgroup-pointer
// signature never survives and `VariablePointers` must NOT be declared. Declaring it for a
// folded-away signature would re-trigger a graphics-driver miscompile (shader-slang/slang#9061),
// so this negative check guards that the capability join is conditional on `[noinline]`.

//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const uint N = 8;
groupshared uint groupSharedData[N];

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

// The helper is inlined away, so neither the boundary nor the capability appears.
// SPV-NOT: OpCapability VariablePointers
// SPV-NOT: OpFunctionCall {{.*}} %writeSlot
