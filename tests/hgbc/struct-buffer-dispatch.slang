//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

public struct BufferPair
{
    public RWStructuredBuffer<uint> first;
    public RWStructuredBuffer<uint> second;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void writePair(uniform BufferPair buffers)
{
    buffers.first[0] = 33;
    buffers.second[0] = 44;
}

[host]
public void main(BufferPair buffers)
{
    dispatch(writePair, uint3(1, 1, 1))(buffers);
}

// CHECK: Section("LITR")
// CHECK-DAG: [[STR_MAIN:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 4 "main"
// CHECK-DAG: [[STR_PAIR:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 10 "BufferPair"
// CHECK-DAG: [[STR_FIRST:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 5 "first"
// CHECK-DAG: [[STR_SECOND:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 6 "second"
// CHECK-DAG: [[STR_WRITE:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 9 "writePair"
// CHECK-DAG: [[BLOB:%[A-Za-z0-9_]+]] = OpBinaryLit {{[0-9]+}} { ... }

// CHECK: Section("EXPT")
// CHECK-DAG: OpExport Function [[STR_MAIN]] {{%[A-Za-z0-9_]+}}
// CHECK-DAG: OpExport Type [[STR_PAIR]] [[PAIR:%[A-Za-z0-9_]+]]

// CHECK: Section("TYPE")
// CHECK-DAG: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType
// CHECK-DAG: [[VOID:%[A-Za-z0-9_]+]] = OpVoidType
// CHECK-DAG: [[BUFFER:%[A-Za-z0-9_]+]] = OpRWStructuredBufferType [[UINT]]
// CHECK-DAG: [[PAIR]] = OpStructType [[STR_FIRST]] [[BUFFER]] [[STR_SECOND]] [[BUFFER]]
// CHECK-DAG: [[FUNC_TYPE:%[A-Za-z0-9_]+]] = OpFuncType [[VOID]] [[PAIR]]
// CHECK-DAG: [[VEC3:%[A-Za-z0-9_]+]] = OpVectorType 3 [[UINT]]
// CHECK-DAG: [[DISPATCH_FUNC_TYPE:%[A-Za-z0-9_]+]] = OpFuncType [[VOID]] [[VEC3]] [[PAIR]]

// CHECK: Section("FUNC")
// CHECK: {{%[A-Za-z0-9_]+}} : [[FUNC_TYPE]] = OpFunc
// CHECK: [[BUFFERS:%[A-Za-z0-9_]+]] : [[PAIR]] = OpParam
// CHECK: OpCall [[DISPATCHER:%[A-Za-z0-9_]+]] {{%[A-Za-z0-9_]+}} [[BUFFERS]]
// CHECK-NOT: OpFieldExtract
// CHECK: OpReturn

// CHECK: Section("EMBD")
// CHECK: {{%[A-Za-z0-9_]+}} = OpSlangModule {{%[A-Za-z0-9_]+}} [[BLOB]]
// CHECK: [[WRITE_ENTRY:%[A-Za-z0-9_]+]] = OpSlangEntryPoint {{%[A-Za-z0-9_]+}} [[STR_WRITE]]
// CHECK: [[DISPATCHER]] : [[DISPATCH_FUNC_TYPE]] = OpMakeDispatcher Compute [[WRITE_ENTRY]]
