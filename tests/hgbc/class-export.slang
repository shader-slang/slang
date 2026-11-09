//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

public class Counter
{
    public uint value;

    [host]
    public __init()
    {
    }

    [host]
    [mutating]
    public void set(uint newValue)
    {
        value = newValue;
    }
}

[host]
public void main()
{
    Counter counter = new Counter();
    counter.set(42);
}

// CHECK-DAG: [[COUNTER_NAME:%[A-Za-z0-9_]+]] : {{.*}} = OpStringLit {{[0-9]+}} "Counter"
// CHECK-DAG: [[VALUE_NAME:%[A-Za-z0-9_]+]] : {{.*}} = OpStringLit {{[0-9]+}} "value"
// CHECK: Section("EXPT")
// CHECK-DAG: OpExport Type [[COUNTER_NAME]] [[COUNTER:%[A-Za-z0-9_]+]]
// CHECK: Section("TYPE")
// CHECK-DAG: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType
// CHECK-DAG: [[COUNTER]] = OpClassType [[VALUE_NAME]] [[UINT]]
