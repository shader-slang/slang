//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

public struct Outer<T>
{
    public struct Inner<U>
    {
        public T outer;
        public U inner;
    }
}

[host]
public Outer<float>.Inner<int> main(Outer<float>.Inner<int> value)
{
    return value;
}

// Each enclosing generic contributes one specialization group in declaration order.
// CHECK-DAG: [[INNER_NAME:%[A-Za-z0-9_]+]] : {{.*}} = OpStringLit {{[0-9]+}} "_SR1N{{[0-9]+}}:_ST{{[^"]*}}5Outerg1T5Innerg1TG1[{{[^"]*}}]G1[{{[^"]*}}"
// CHECK-DAG: OpDecorate TypeMangledName [[INNER:%[A-Za-z0-9_]+]] [[INNER_NAME]]
// CHECK-DAG: OpFuncType [[INNER]] [[INNER]]
