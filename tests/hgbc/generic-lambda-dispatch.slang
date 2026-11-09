//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[shader("compute")]
[numthreads(1, 1, 1)]
void mapBufferInPlaceCS<
    T : __BuiltinArithmeticType,
    TFunc : IFunc<T, T>>(
    uint tid : SV_DispatchThreadID,
    RWStructuredBuffer<T> values,
    uint count,
    TFunc transform)
{
    if (tid < count)
        values[tid] = transform(values[tid]);
}

void mapInPlace<
    T : __BuiltinArithmeticType,
    TFunc : IFunc<T, T>>(
    RWStructuredBuffer<T> values,
    uint count,
    TFunc transform)
{
    dispatch(mapBufferInPlaceCS<T, TFunc>, uint3(count, 1, 1))(
        values,
        count,
        transform);
}

void addInPlace<T : __BuiltinArithmeticType>(
    RWStructuredBuffer<T> values,
    T amount)
{
    uint count;
    uint stride;
    values.GetDimensions(count, stride);
    mapInPlace(values, count, (T value) => value + amount);
}

[host]
public void main(RWStructuredBuffer<float> values, float amount)
{
    addInPlace<float>(values, amount);
}

// CHECK: OpStringLit {{[0-9]+}} "addInPlace<float>._slang_Lambda_addInPlace_2"
