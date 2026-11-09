//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

void setValue(out uint target, uint source)
{
    target = source;
}

void increment(inout uint target, uint amount)
{
    target += amount;
}

[noinline]
uint readBorrow(__constref uint source)
{
    return source;
}

struct Counter
{
    uint value;

    [mutating]
    void add(uint amount)
    {
        value += amount;
    }
}

[host]
public uint main(uint input)
{
    uint value;
    setValue(value, input);
    increment(value, 2);

    Counter counter;
    counter.value = value;
    counter.add(3);
    uint borrowValue = readBorrow(value);
    return counter.value + borrowValue;
}

// CHECK: Section("TYPE")
// CHECK-DAG: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType
// CHECK-DAG: [[VOID:%[A-Za-z0-9_]+]] = OpVoidType
// CHECK-DAG: [[COUNTER:%[A-Za-z0-9_]+]] = OpStructType
// CHECK-DAG: [[PTR_UINT:%[A-Za-z0-9_]+]] = OpPtrType [[UINT]]
// CHECK-DAG: [[PTR_COUNTER:%[A-Za-z0-9_]+]] = OpPtrType [[COUNTER]]
// CHECK-DAG: [[OUT_UINT:%[A-Za-z0-9_]+]] = OpOutType [[UINT]]
// CHECK-DAG: [[INOUT_UINT:%[A-Za-z0-9_]+]] = OpInOutType [[UINT]]
// CHECK-DAG: [[INOUT_COUNTER:%[A-Za-z0-9_]+]] = OpInOutType [[COUNTER]]
// CHECK-DAG: [[BORROW_UINT:%[A-Za-z0-9_]+]] = OpBorrowInType [[UINT]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} = OpFuncType [[VOID]] [[OUT_UINT]] [[UINT]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} = OpFuncType [[VOID]] [[INOUT_UINT]] [[UINT]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} = OpFuncType [[VOID]] [[INOUT_COUNTER]] [[UINT]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} = OpFuncType [[UINT]] [[BORROW_UINT]]

// CHECK: Section("FUNC")
// CHECK: [[INPUT:%[A-Za-z0-9_]+]] : [[UINT]] = OpParam
// CHECK: [[VALUE_REF:%[A-Za-z0-9_]+]] : [[PTR_UINT]] = OpVar
// CHECK: OpCall {{%[A-Za-z0-9_]+}} [[VALUE_REF]] [[INPUT]]
// CHECK: OpCall {{%[A-Za-z0-9_]+}} [[VALUE_REF]] {{%[A-Za-z0-9_]+}}
// CHECK: [[COUNTER_REF:%[A-Za-z0-9_]+]] : [[PTR_COUNTER]] = OpVar
// CHECK: {{%[A-Za-z0-9_]+}} : [[PTR_UINT]] = OpFieldAddress [[COUNTER_REF]]
// CHECK: OpCall {{%[A-Za-z0-9_]+}} [[COUNTER_REF]] {{%[A-Za-z0-9_]+}}
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[OUT_UINT]] = OpParam
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[INOUT_UINT]] = OpParam
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[INOUT_COUNTER]] = OpParam
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[BORROW_UINT]] = OpParam
