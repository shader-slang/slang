//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main
//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -obfuscate

public struct Holder<T>
{
    public T value;
}

// Constraint witnesses from _Texture's source declaration must not become extra IR operands.
// Otherwise the holder loses its portable identity and cannot cross loaded module boundaries.
[host]
public Holder<RWTexture2D<float4>> main(Holder<RWTexture2D<float4>> value)
{
    return value;
}

// CHECK-DAG: [[NAME:%[A-Za-z0-9_]+]] : {{.*}} = OpStringLit {{[0-9]+}} "_SR1{{[^"]*}}6Holderg1TG1[{{[^"]*}}"
// CHECK-DAG: OpDecorate TypeMangledName [[HOLDER:%[A-Za-z0-9_]+]] [[NAME]]
// CHECK-DAG: OpFuncType [[HOLDER]] [[HOLDER]]
