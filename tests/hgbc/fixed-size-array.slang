//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

Array<uint, 4> makeValues(uint base)
{
    return __makeArray(base, base + 1, base + 2, base + 3);
}

[host]
public uint main(uint base, uint index)
{
    Array<uint, 4> values = makeValues(base);
    values[index] = 100;
    return values[2];
}

// CHECK: Section("TYPE")
// CHECK-DAG: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType
// CHECK-DAG: [[ARRAY:%[A-Za-z0-9_]+]] = OpArrayType 4 [[UINT]]
// CHECK-DAG: [[PTR_UINT:%[A-Za-z0-9_]+]] = OpPtrType [[UINT]]
// CHECK-DAG: [[PTR_ARRAY:%[A-Za-z0-9_]+]] = OpPtrType [[ARRAY]]

// CHECK: Section("FUNC")
// CHECK-DAG: OpMakeArray
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[PTR_ARRAY]] = OpVar
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[PTR_UINT]] = OpElementAddress
