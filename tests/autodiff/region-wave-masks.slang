//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -output-using-type 

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;


void doThing(uint thread_idx, uint out_size, uint x, out uint biases[128])
{
    if (thread_idx < out_size) {
        biases[thread_idx] = thread_idx * x;
    }
    GroupMemoryBarrierWithGroupSync();
    AllMemoryBarrierWithWaveMaskSync(0xFFFFFFFF);
}

[CudaKernel]
[numthreads(128, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_GroupThreadID)
{
    uint idx = dispatchThreadID.x;

    uint biases[128];
    doThing(idx, 128, 1, biases);

    AllMemoryBarrierWithWaveMaskSync(0xFFFFFFFF);

    uint g = biases[idx];
    uint sum = WaveActiveSum(g);

    if (idx < 4)
        outputBuffer[idx] = biases[idx];
 
    uint val = 1;
    if (WaveIsFirstLane())
    {
        val = 1;
    }
    uint totalVals = WaveActiveSum(val);

    if (WaveIsFirstLane())
    {
        uint oldVal;
        InterlockedExchange(outputBuffer[idx / 32], totalVals, oldVal);
    }

}
