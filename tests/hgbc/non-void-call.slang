//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

uint passThrough(uint value)
{
    return value;
}

[host]
public void main(uint value)
{
    let result = passThrough(value);
    print(result);
}

// CHECK: Section("TYPE")
// CHECK-DAG: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType

// CHECK: Section("FUNC")
// CHECK: [[VALUE:%[A-Za-z0-9_]+]] : [[UINT]] = OpParam
// CHECK: [[CALL:%[A-Za-z0-9_]+]] : [[UINT]] = OpCall [[PASS_THROUGH:%[A-Za-z0-9_]+]] [[VALUE]]
// CHECK: OpPrint [[UINT]] [[CALL]]
// CHECK: OpReturn
// CHECK: [[HELPER_VALUE:%[A-Za-z0-9_]+]] : [[UINT]] = OpParam
// CHECK: OpReturn [[HELPER_VALUE]]
