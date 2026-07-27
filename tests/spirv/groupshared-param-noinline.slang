// A bare `groupshared` array parameter on a `[noinline]` function is passed by reference. On the
// direct-SPIR-V target the call boundary is kept (the function is NOT inlined away), and the
// surviving Workgroup-pointer parameter requires the `VariablePointers` capability /
// `SPV_KHR_variable_pointers` extension (shader-slang/slang#10641). See
// groupshared-param-noinline-inline-negative.slang for the contrasting force-inlined case.

//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
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

// The variable-pointers capability/extension follow the surviving Workgroup-pointer signature.
// SPV-DAG: OpCapability VariablePointers
// SPV-DAG: OpExtension "SPV_KHR_variable_pointers"
// The `[noinline]` boundary is kept: the helper is a real function that is called, not inlined.
// SPV: OpFunctionCall {{.*}} %writeSlot
