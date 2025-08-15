[shader("compute")]
[numthreads(4, 1, 1)]
void times(
    uint3 sv_dispatchThreadID: SV_DispatchThreadID,
    uniform RWStructuredBuffer<float> buffer)
{
    var input = buffer[sv_dispatchThreadID.x];
    buffer[sv_dispatchThreadID.x] = sv_dispatchThreadID.x;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void plus(
    uint3 sv_dispatchThreadID: SV_DispatchThreadID,
    uniform RWStructuredBuffer<float> buffer)
{
    var input = buffer[sv_dispatchThreadID.x];
    buffer[sv_dispatchThreadID.x] = sv_dispatchThreadID.x;
}
