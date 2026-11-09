//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main
//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -obfuscate

public interface IValue
{
}

public struct Item<T> : IValue
{
    public T value;
}

public struct Wrapper<T : IValue>
{
    public T value;
}

[host]
public Wrapper<Item<float>> main(Wrapper<Item<float>> value)
{
    return value;
}

// The lowered type keeps its nominal argument; unused constraint witnesses are erased. The
// complete argument recipe remains available independently of code-linkage obfuscation.
// CHECK-DAG: [[WRAPPER_NAME:%[A-Za-z0-9_]+]] : {{.*}} = OpStringLit {{[0-9]+}} "_SR1N{{[0-9]+}}:_ST{{[^"]*}}7Wrapperg{{[^"]*}}G1[N{{[0-9]+}}:_SR1{{[^"]*}}4Itemg1TG1[{{[^"]*}}"
// CHECK-DAG: OpDecorate TypeMangledName [[WRAPPER:%[A-Za-z0-9_]+]] [[WRAPPER_NAME]]
// CHECK-DAG: OpFuncType [[WRAPPER]] [[WRAPPER]]
