//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[host]
public uint main(bool condition, uint left, uint right)
{
    uint selected;
    if (condition)
        selected = left;
    else
        selected = right;
    return selected;
}

// CHECK: Section("TYPE")
// CHECK-DAG: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType
// CHECK-DAG: [[PTR_UINT:%[A-Za-z0-9_]+]] = OpPtrType [[UINT]]
// CHECK: Section("FUNC")
// CHECK: [[FUNCTION:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpFunc
// CHECK: [[ENTRY:%[A-Za-z0-9_]+]] = OpBlock
// CHECK: [[CONDITION:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpParam
// CHECK: [[LEFT:%[A-Za-z0-9_]+]] : [[UINT]] = OpParam
// CHECK: [[RIGHT:%[A-Za-z0-9_]+]] : [[UINT]] = OpParam
// CHECK: [[SELECTED:%[A-Za-z0-9_]+]] : [[PTR_UINT]] = OpVar
// CHECK: OpBranchConditional [[CONDITION]] [[THEN:%[A-Za-z0-9_]+]] [[ELSE:%[A-Za-z0-9_]+]]
// CHECK: [[THEN]] = OpBlock
// CHECK-NOT: OpParam
// CHECK: OpStore [[SELECTED]] [[LEFT]]
// CHECK: OpBranch [[MERGE:%[A-Za-z0-9_]+]]
// CHECK: [[ELSE]] = OpBlock
// CHECK-NOT: OpParam
// CHECK: OpStore [[SELECTED]] [[RIGHT]]
// CHECK: OpBranch [[MERGE]]
// CHECK: [[MERGE]] = OpBlock
// CHECK-NOT: OpParam
// CHECK: [[RESULT:%[A-Za-z0-9_]+]] : [[UINT]] = OpLoad [[SELECTED]]
// CHECK: OpReturn [[RESULT]]
