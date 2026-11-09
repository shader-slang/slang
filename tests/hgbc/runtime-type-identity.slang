//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main
//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -obfuscate

public struct Point
{
    public float x;
    public float y;
}

public struct Wrapper<T>
{
    public T value;
}

[host]
public Wrapper<Point[2]> main(Wrapper<Point[2]> value)
{
    return value;
}

// CHECK-DAG: [[POINT_NAME:%[A-Za-z0-9_]+]] : {{.*}} = OpStringLit {{[0-9]+}} "_ST{{[^"]*}}5Point"
// CHECK-DAG: [[WRAPPER_NAME:%[A-Za-z0-9_]+]] : {{.*}} = OpStringLit {{[0-9]+}} "_SR1N{{[0-9]+}}:_ST{{[^"]*}}7Wrapperg1TG1[{{[^"]*}}"
// CHECK-DAG: OpDecorate TypeMangledName [[POINT:%[A-Za-z0-9_]+]] [[POINT_NAME]]
// CHECK-DAG: OpDecorate TypeMangledName [[WRAPPER:%[A-Za-z0-9_]+]] [[WRAPPER_NAME]]
// CHECK-DAG: [[POINT]] = OpStructType
// CHECK-DAG: [[WRAPPER]] = OpStructType
