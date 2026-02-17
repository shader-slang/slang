#pragma warning(disable: 41017)


public bool equals<T>(T a, T b) where T : __BuiltinFloatingPointType
{
    return abs(a - b) < T(1e-2f);
}

public void initBias<T, int Size, int WorkgroupSize = 32>(uint tid, RWStructuredBuffer<T> biasBuffer)
    where T : __BuiltinFloatingPointType
{
    static const int NumIters = (Size + WorkgroupSize - 1) / WorkgroupSize;
    for (int iter = 0; iter < NumIters; iter++)
    {
        int index = iter * WorkgroupSize + tid;
        if (index < Size)
            biasBuffer[index] = T(index);
    }

    AllMemoryBarrierWithGroupSync();
}

public void initBias<T, int Size, int WorkgroupSize = 32>(uint tid, T* biasBuffer)
    where T : __BuiltinFloatingPointType
{
    static const int NumIters = (Size + WorkgroupSize - 1) / WorkgroupSize;
    for (int iter = 0; iter < NumIters; iter++)
    {
        int index = iter * WorkgroupSize + tid;
        if (index < Size)
            biasBuffer[index] = T(index);
    }

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

// Pad a size up to the next multiple of 16 (cooperative matrix tile size).
public struct PadToTile<int Size>
{
    public static const int Value = ((Size + 15) / 16) * 16;
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
