import gpu_state;

[shader("compute")]
[numthreads(64, 1, 1)]
void increment(
    RWStructuredBuffer<uint> values,
    uint count,
    uint amount,
    uint3 thread : SV_DispatchThreadID)
{
    if (thread.x < count)
        values[thread.x] += amount;
}

[host]
GpuState copyState(GpuState state)
{
    // Copying the struct retains the same GPU buffer.
    return state;
}

[host]
void add(GpuState state, uint amount)
{
    dispatch(increment, uint3((state.count + 63) / 64, 1, 1))(
        state.values, state.count, amount);
}

[host]
uint sum(GpuState state)
{
    uint total = 0;
    for (uint i = 0; i < state.count; ++i)
        total += state.values.readBack(i);
    return total;
}

[host]
uint readValue(RWStructuredBuffer<uint> values, uint index) { return values.readBack(index); }
