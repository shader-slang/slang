#pragma warning(disable: 41017)


public bool equals<T>(T a, T b) where T : __BuiltinFloatingPointType
{
    return abs(a - b) < T(1e-2f);
}

public void initBias<T, int Size>(uint tid, RWStructuredBuffer<T> biasBuffer)
    where T : __BuiltinFloatingPointType
{
    if (tid < Size)
        biasBuffer[tid] = T(tid);

    AllMemoryBarrierWithGroupSync();
}

public bool collectResults<int BatchSize>(uint tid, bool perThreadResult)
{
    static groupshared bool s_resultMem[BatchSize / 32];

    // each warp will check the result
    bool res = WaveActiveAllTrue(perThreadResult);

    int subgroupIndex = tid / 32;
    int laneId = WaveGetLaneIndex();
    // use first lane of each warp to write the result to the shared memory so that
    // we can reduce the result further
    if (laneId == 0)
    {
        // Note max subgroupIndex is "BatchSize/32 - 1"
        s_resultMem[subgroupIndex] = res;
    }

    AllMemoryBarrierWithGroupSync();

    int warpCount = BatchSize / 32;
    bool finalResult = true;
    // We will use the "warpCount" threads in the first warp to reduce the result.
    if (tid < warpCount)
    {
        finalResult = s_resultMem[tid];
    }

    finalResult = WaveActiveAllTrue(finalResult);

    return finalResult;
}

public void setBufferOneValue<int Size, int WorkgroupSize, T>(uint tid, RWStructuredBuffer<T> buffer, T value)
    where T : __BuiltinFloatingPointType
{
    int numIter = (Size + WorkgroupSize - 1) / WorkgroupSize;
    for (int i = 0; i < numIter; i++)
    {
        int index = i * WorkgroupSize + tid;
        if (index >= Size)
            break;

        buffer[index] = value;
    }

    AllMemoryBarrierWithGroupSync();
}
