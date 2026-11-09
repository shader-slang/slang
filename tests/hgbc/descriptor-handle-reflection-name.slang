//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main
//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -obfuscate

// These resources have the same host element size and stride. Their handle types must retain
// distinct semantic identities so runtime shader argument checks preserve the layout parameter.
[host]
void main(out RWStructuredBuffer<uint>.Handle defaultHandle,
    out RWStructuredBuffer<uint, ScalarDataLayout>.Handle scalarHandle)
{
    defaultHandle = RWStructuredBuffer<uint>::alloc(1).getDescriptorHandle();
    scalarHandle = RWStructuredBuffer<uint, ScalarDataLayout>::alloc(1).getDescriptorHandle();
}

// CHECK-DAG: [[DEFAULT_NAME:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit {{[0-9]+}} "DescriptorHandle<RWStructuredBuffer<uint, DefaultDataLayout>>"
// CHECK-DAG: [[SCALAR_NAME:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit {{[0-9]+}} "DescriptorHandle<RWStructuredBuffer<uint, ScalarDataLayout>>"
// CHECK-DAG: [[DEFAULT_TYPE:%[A-Za-z0-9_]+]] = OpDescriptorHandleType
// CHECK-DAG: [[SCALAR_TYPE:%[A-Za-z0-9_]+]] = OpDescriptorHandleType
// CHECK-DAG: OpDecorate SlangTypeName [[DEFAULT_TYPE]] [[DEFAULT_NAME]]
// CHECK-DAG: OpDecorate SlangTypeName [[SCALAR_TYPE]] [[SCALAR_NAME]]
