//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[host]
public void main(bool value)
{
    print(value);
}

// CHECK: Section("TYPE")
// CHECK-DAG: [[BOOL:%[A-Za-z0-9_]+]] = OpBoolType
// CHECK-DAG: [[VOID:%[A-Za-z0-9_]+]] = OpVoidType
// CHECK-DAG: [[FUNC_TYPE:%[A-Za-z0-9_]+]] = OpFuncType [[VOID]] [[BOOL]]

// CHECK: Section("FUNC")
// CHECK: {{%[A-Za-z0-9_]+}} : [[FUNC_TYPE]] = OpFunc
// CHECK: {{%[A-Za-z0-9_]+}} = OpBlock
// CHECK: [[VALUE:%[A-Za-z0-9_]+]] : [[BOOL]] = OpParam
// CHECK: OpPrint [[BOOL]] [[VALUE]]
// CHECK: OpReturn
// CHECK: OpBlockEnd
// CHECK: OpFunctionEnd
