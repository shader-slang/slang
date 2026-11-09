//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[host]
public void main(uint x, uint y)
{
    print(x < y);
    print(x > y);
    print(x <= y);
    print(x >= y);
}

// CHECK: Section("TYPE")
// CHECK-DAG: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType
// CHECK-DAG: [[BOOL:%[A-Za-z0-9_]+]] = OpBoolType
// CHECK-DAG: [[VOID:%[A-Za-z0-9_]+]] = OpVoidType
// CHECK-DAG: [[FUNC_TYPE:%[A-Za-z0-9_]+]] = OpFuncType [[VOID]] [[UINT]] [[UINT]]

// CHECK: Section("FUNC")
// CHECK: {{%[A-Za-z0-9_]+}} : [[FUNC_TYPE]] = OpFunc
// CHECK: {{%[A-Za-z0-9_]+}} = OpBlock
// CHECK: [[X:%[A-Za-z0-9_]+]] : [[UINT]] = OpParam
// CHECK: [[Y:%[A-Za-z0-9_]+]] : [[UINT]] = OpParam
// CHECK: [[LESS:%[A-Za-z0-9_]+]] : [[BOOL]] = OpLess [[UINT]] [[X]] [[Y]]
// CHECK: OpPrint [[BOOL]] [[LESS]]
// CHECK: [[GREATER:%[A-Za-z0-9_]+]] : [[BOOL]] = OpGreater [[UINT]] [[X]] [[Y]]
// CHECK: OpPrint [[BOOL]] [[GREATER]]
// CHECK: [[LEQ:%[A-Za-z0-9_]+]] : [[BOOL]] = OpLeq [[UINT]] [[X]] [[Y]]
// CHECK: OpPrint [[BOOL]] [[LEQ]]
// CHECK: [[GEQ:%[A-Za-z0-9_]+]] : [[BOOL]] = OpGeq [[UINT]] [[X]] [[Y]]
// CHECK: OpPrint [[BOOL]] [[GEQ]]
// CHECK: OpReturn
// CHECK: OpBlockEnd
// CHECK: OpFunctionEnd
