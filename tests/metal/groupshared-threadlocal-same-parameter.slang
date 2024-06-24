//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-slang -shaderobj -mtl

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static groupshared uint g_values[2] = { 0, 1 };
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
    outputBuffer[tid] = g_valuesReturned[tid];
    // BUF: 0
    // BUF: 3
}
