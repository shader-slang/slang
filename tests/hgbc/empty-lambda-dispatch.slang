//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[shader("compute")]
[numthreads(1, 1, 1)]
void mapBufferInPlaceCS<TFunc : IFunc<float, float>>(
    uint tid : SV_DispatchThreadID,
    RWStructuredBuffer<float> values,
    uint count,
    TFunc transform)
{
    if (tid < count)
        values[tid] = transform(values[tid]);
}

void mapInPlace<TFunc : IFunc<float, float>>(
    RWStructuredBuffer<float> values,
    uint count,
    TFunc transform)
{
    dispatch(mapBufferInPlaceCS<TFunc>, uint3(1, 1, 1))(values, count, transform);
}

[host]
public void main(RWStructuredBuffer<float> values)
{
    // The identity lambda has an empty closure, so its dispatch argument is erased by type
    // legalization.
    mapInPlace(values, 1, (float value) => value);
}

// CHECK: OpMakeDispatcher Compute
