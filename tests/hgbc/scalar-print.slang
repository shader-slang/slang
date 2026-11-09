//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[host]
public void main(uint x, uint y)
{
    print(x);
    print(y);
}

// CHECK: Section("TYPE")
// CHECK: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType
// CHECK: [[VOID:%[A-Za-z0-9_]+]] = OpVoidType
// CHECK: [[MAIN_TYPE:%[A-Za-z0-9_]+]] = OpFuncType [[VOID]] [[UINT]] [[UINT]]
// CHECK: Section("FUNC")
// CHECK: {{%[A-Za-z0-9_]+}} : [[MAIN_TYPE]] = OpFunc
// CHECK: [[X:%[A-Za-z0-9_]+]] : [[UINT]] = OpParam
// CHECK: [[Y:%[A-Za-z0-9_]+]] : [[UINT]] = OpParam
// CHECK: OpPrint [[UINT]] [[X]]
// CHECK: OpPrint [[UINT]] [[Y]]
// CHECK: OpReturn
// CHECK: OpBlockEnd
// CHECK: OpFunctionEnd
