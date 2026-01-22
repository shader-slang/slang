[shader("compute")]
[numthreads(4, 1, 1)]
void times(
    uint3 sv_dispatchThreadID: SV_DispatchThreadID,
    uniform float a,
    uniform StructuredBuffer<float> x,
    uniform RWStructuredBuffer<float> ax)
{
    uint i = sv_dispatchThreadID.x;
    ax[i] = a * x[i];
}

[shader("compute")]
[numthreads(4, 1, 1)]
void plus(
    uint3 sv_dispatchThreadID: SV_DispatchThreadID,
    uniform StructuredBuffer<float> ax,
    uniform StructuredBuffer<float> y,
    uniform RWStructuredBuffer<float> axpy)
{
    uint i = sv_dispatchThreadID.x;
    axpy[i] = ax[i] + y[i];
}
