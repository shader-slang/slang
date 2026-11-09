//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

public struct Box<T>
{
    T value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void clear<T>(RWStructuredBuffer<T> values)
{
    values[0] = T();
}

[host]
public void main(RWStructuredBuffer<Box<float>> values)
{
    dispatch(clear<Box<float>>, uint3(1, 1, 1))(values);
}

// CHECK-DAG: [[BOX_DEBUG_NAME:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit {{[0-9]+}} "Box"
// CHECK-DAG: [[BOX_REFLECTION_NAME:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit {{[0-9]+}} "Box<float>"
// CHECK-DAG: OpDecorate NameHint [[BOX_TYPE:%[A-Za-z0-9_]+]] [[BOX_DEBUG_NAME]]
// CHECK-DAG: OpDecorate SlangTypeName [[BOX_TYPE]] [[BOX_REFLECTION_NAME]]
