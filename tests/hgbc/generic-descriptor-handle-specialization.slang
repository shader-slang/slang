//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

// A generic host method that takes a descriptor handle whose type depends on the generic parameter
// specializes into ordinary host code. Consider `Holder<T>.run` below: lowering gives the type
// `DescriptorHandle<RWStructuredBuffer<T>>` a `ReflectionName` decoration whose string is computed
// from `T`, and that string can come after the type in the body of the generic. Specialization
// therefore clones the string before the decoration that names it. Otherwise the name of the
// specialized handle type keeps referring into the unspecialized `Holder<T>.run`, which keeps that
// generic alive, and the `static_assert` it still reaches through `checkedCount<4, T>` is reported
// as "static assertion condition not compile-time constant".

[host]
uint checkedCount<let N : int, U>()
{
    static_assert(N > 0, "N must be positive");
    return uint(N);
}

struct Holder<T>
{
    [host]
    static uint run()
    {
        let handle = RWStructuredBuffer<T>::alloc(1).getDescriptorHandle();
        return checkedCount<4, T>();
    }
}

[host]
void main(out uint count)
{
    count = Holder<float>.run();
}

// The specialized handle type carries its concrete name rather than a reference into the generic.
// CHECK-DAG: [[NAME:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit {{[0-9]+}} "DescriptorHandle<RWStructuredBuffer<float, DefaultDataLayout>>"
// CHECK-DAG: [[TYPE:%[A-Za-z0-9_]+]] = OpDescriptorHandleType
// CHECK-DAG: OpDecorate SlangTypeName [[TYPE]] [[NAME]]
