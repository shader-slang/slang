//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[host]
public void main()
{
    print("hello world");
}

// CHECK: Section("LITR")
// CHECK: %strMain : [[STRING:%[A-Za-z0-9_]+]] = OpStringLit 4 "main"
// CHECK: %strHelloWorld : [[STRING]] = OpStringLit 11 "hello world"
// CHECK: Section("EXPT")
// CHECK: OpExport Function %strMain %main
// CHECK: Section("META")
// CHECK: OpDecorate NameHint %main %strMain
// CHECK: Section("TYPE")
// CHECK: [[VOID:%[A-Za-z0-9_]+]] = OpVoidType
// CHECK: %mainFuncType = OpFuncType [[VOID]]
// CHECK: Section("FUNC")
// CHECK: %main : %mainFuncType = OpFunc
// CHECK: %mainBlock = OpBlock
// CHECK: OpPrint [[STRING]] %strHelloWorld
// CHECK: OpReturn
// CHECK: OpBlockEnd
// CHECK: OpFunctionEnd
