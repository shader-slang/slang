//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[host]
public void main(bool useThen)
{
    if (useThen)
    {
        print("then");
    }
    else
    {
        print("else");
    }
    print("done");
}

// CHECK: Section("LITR")
// CHECK-DAG: [[STR_THEN:%[A-Za-z0-9_]+]] : [[STRING:%[A-Za-z0-9_]+]] = OpStringLit 4 "then"
// CHECK-DAG: [[STR_ELSE:%[A-Za-z0-9_]+]] : [[STRING]] = OpStringLit 4 "else"
// CHECK-DAG: [[STR_DONE:%[A-Za-z0-9_]+]] : [[STRING]] = OpStringLit 4 "done"

// CHECK: Section("TYPE")
// CHECK-DAG: [[BOOL:%[A-Za-z0-9_]+]] = OpBoolType
// CHECK-DAG: [[VOID:%[A-Za-z0-9_]+]] = OpVoidType
// CHECK-DAG: [[FUNC_TYPE:%[A-Za-z0-9_]+]] = OpFuncType [[VOID]] [[BOOL]]

// CHECK: Section("FUNC")
// CHECK: {{%[A-Za-z0-9_]+}} : [[FUNC_TYPE]] = OpFunc
// CHECK: [[ENTRY:%[A-Za-z0-9_]+]] = OpBlock
// CHECK: [[COND:%[A-Za-z0-9_]+]] : [[BOOL]] = OpParam
// CHECK: OpBranchConditional [[COND]] [[THEN_BLOCK:%[A-Za-z0-9_]+]] [[ELSE_BLOCK:%[A-Za-z0-9_]+]]
// CHECK: OpBlockEnd
// CHECK: [[THEN_BLOCK]] = OpBlock
// CHECK: OpPrint [[STRING]] [[STR_THEN]]
// CHECK: OpBranch [[AFTER_BLOCK:%[A-Za-z0-9_]+]]
// CHECK: OpBlockEnd
// CHECK: [[ELSE_BLOCK]] = OpBlock
// CHECK: OpPrint [[STRING]] [[STR_ELSE]]
// CHECK: OpBranch [[AFTER_BLOCK]]
// CHECK: OpBlockEnd
// CHECK: [[AFTER_BLOCK]] = OpBlock
// CHECK: OpPrint [[STRING]] [[STR_DONE]]
// CHECK: OpReturn
// CHECK: OpBlockEnd
// CHECK: OpFunctionEnd
