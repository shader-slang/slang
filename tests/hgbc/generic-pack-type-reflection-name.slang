//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

public struct ValuePackType<let Head : int, let each Tail : int>
{
    int value;
}

public struct TypePackType<Head, each Tail>
{
    int value;
}

public struct TwoTypePacks<each First, each Second>
{
    int value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void clear<T>(RWStructuredBuffer<T> values)
{
    values[0] = T();
}

[host]
public void main(
    RWStructuredBuffer<ValuePackType<1>> emptyValuePack,
    RWStructuredBuffer<ValuePackType<1, 2, 3>> nonEmptyValuePack,
    RWStructuredBuffer<TypePackType<int>> emptyTypePack,
    RWStructuredBuffer<TypePackType<int, float, bool>> nonEmptyTypePack,
    RWStructuredBuffer<TwoTypePacks<int, float>> twoTypePacks)
{
    dispatch(clear<ValuePackType<1>>, uint3(1, 1, 1))(emptyValuePack);
    dispatch(clear<ValuePackType<1, 2, 3>>, uint3(1, 1, 1))(nonEmptyValuePack);
    dispatch(clear<TypePackType<int>>, uint3(1, 1, 1))(emptyTypePack);
    dispatch(clear<TypePackType<int, float, bool>>, uint3(1, 1, 1))(nonEmptyTypePack);
    dispatch(clear<TwoTypePacks<int, float>>, uint3(1, 1, 1))(twoTypePacks);
}

// CHECK-DAG: OpStringLit {{[0-9]+}} "ValuePackType<1>"
// CHECK-DAG: OpStringLit {{[0-9]+}} "ValuePackType<1, 2, 3>"
// CHECK-DAG: OpStringLit {{[0-9]+}} "TypePackType<int>"
// CHECK-DAG: OpStringLit {{[0-9]+}} "TypePackType<int, float, bool>"
// CHECK-DAG: OpStringLit {{[0-9]+}} "TwoTypePacks<int, float>"
