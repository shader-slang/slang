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
