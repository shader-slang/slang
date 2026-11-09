import gpu_state;

[shader("compute")]
[numthreads(64, 1, 1)]
void initialize(
    RWStructuredBuffer<uint> values,
    uint count,
    uint seed,
    uint3 thread : SV_DispatchThreadID)
{
    if (thread.x < count)
        values[thread.x] = seed + thread.x;
}

[host]
GpuState makeState(uint count, uint seed)
{
    let values = RWStructuredBuffer<uint>::alloc(count);
    dispatch(initialize, uint3((count + 63) / 64, 1, 1))(values, count, seed);
    return GpuState(values, count);
}

[host]
RWStructuredBuffer<uint> getBuffer(GpuState state) { return state.values; }
