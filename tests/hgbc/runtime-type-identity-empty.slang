//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

public struct Empty
{
}

public struct Container
{
    public Empty empty;
    public float value;
}

[host]
public Empty main(Container value, Empty result)
{
    return result;
}

[host]
public Empty makeEmpty()
{
    return Empty();
}

// CHECK-DAG: [[EMPTY_NAME:%[A-Za-z0-9_]+]] : {{.*}} = OpStringLit {{[0-9]+}} "_ST{{[^"]*}}5Empty"
// CHECK-DAG: [[CONTAINER_NAME:%[A-Za-z0-9_]+]] : {{.*}} = OpStringLit {{[0-9]+}} "_ST{{[^"]*}}9Container"
// CHECK-DAG: OpDecorate TypeMangledName [[EMPTY:%[A-Za-z0-9_]+]] [[EMPTY_NAME]]
// CHECK-DAG: OpDecorate TypeMangledName [[CONTAINER:%[A-Za-z0-9_]+]] [[CONTAINER_NAME]]
// CHECK-DAG: [[EMPTY]] = OpStructType
// CHECK-DAG: [[CONTAINER]] = OpStructType {{%[A-Za-z0-9_]+}} [[EMPTY]]
// CHECK-DAG: OpFuncType [[EMPTY]] [[CONTAINER]] [[EMPTY]]
