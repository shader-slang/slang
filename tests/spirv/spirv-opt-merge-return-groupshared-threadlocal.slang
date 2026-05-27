//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-via-glsl
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly

// This is the reduced input from #11146. These tests are compile-smoke coverage
// for both non-GPU SPIR-V emission paths. The CMake option gate itself is checked
// by extras/check-spirv-opt-merge-return-workaround.sh so the same test can pass
// with SLANG_ENABLE_SPIRV_OPT_MERGE_RETURN=ON default builds and PR CI's
// temporary OFF artifact builds.

RWStructuredBuffer<uint> outputBuffer;

static groupshared uint g_values[2];
static uint g_altValues[2] = { 2, 3 };

static groupshared uint g_valuesReturned[2];
static uint g_altValuesReturned[2];

uint[2] maybeGroupSharedReturn(uint id)
{
    if (id == 0)
    {
        return g_values;
    }
    else
    {
        return g_altValues;
    }
}

[numthreads(2, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // SPIRV: OpEntryPoint GLCompute {{.*}} "computeMain"
    // SPIRV: OpExecutionMode {{.*}} LocalSize 2 1 1

    g_values = { 1, 0 };
    AllMemoryBarrierWithGroupSync();
    uint tid = dispatchThreadID.x;
    if (tid == 0)
    {
        g_valuesReturned[tid] = maybeGroupSharedReturn(tid)[tid];
    }
    else
    {
        g_altValuesReturned[tid] = maybeGroupSharedReturn(tid)[tid];
    }

    AllMemoryBarrierWithGroupSync();
    if (tid == 0)
    {
        outputBuffer[tid] = g_valuesReturned[tid];
    }
    else
    {
        outputBuffer[tid] = g_altValuesReturned[tid];
    }
}
