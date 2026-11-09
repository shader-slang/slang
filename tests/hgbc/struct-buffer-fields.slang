//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

public struct BufferPair
{
    public RWStructuredBuffer<uint> first;
    public RWStructuredBuffer<uint> second;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void writeFirst(RWStructuredBuffer<uint> output)
{
    output[0] = 11;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void writeSecond(RWStructuredBuffer<uint> output)
{
    output[0] = 22;
}

[host]
public void main(BufferPair buffers)
{
    dispatch(writeFirst, uint3(1, 1, 1))(buffers.first);
    dispatch(writeSecond, uint3(1, 1, 1))(buffers.second);
}

// CHECK: Section("LITR")
// CHECK-DAG: [[STR_MAIN:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 4 "main"
// CHECK-DAG: [[STR_PAIR:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 10 "BufferPair"
// CHECK-DAG: [[STR_FIRST:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 5 "first"
// CHECK-DAG: [[STR_SECOND:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 6 "second"

// CHECK: Section("EXPT")
// CHECK-DAG: OpExport Function [[STR_MAIN]] {{%[A-Za-z0-9_]+}}
// CHECK-DAG: OpExport Type [[STR_PAIR]] [[PAIR:%[A-Za-z0-9_]+]]

// CHECK: Section("TYPE")
// CHECK-DAG: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType
// CHECK-DAG: [[BUFFER:%[A-Za-z0-9_]+]] = OpRWStructuredBufferType [[UINT]]
// CHECK-DAG: [[PAIR]] = OpStructType [[STR_FIRST]] [[BUFFER]] [[STR_SECOND]] [[BUFFER]]

// CHECK: Section("FUNC")
// CHECK: [[BUFFERS:%[A-Za-z0-9_]+]] : [[PAIR]] = OpParam
// CHECK: [[FIRST:%[A-Za-z0-9_]+]] : [[BUFFER]] = OpFieldExtract [[BUFFERS]] [[STR_FIRST]]
// CHECK: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpCall [[FIRST_DISPATCHER:%[A-Za-z0-9_]+]] {{%[A-Za-z0-9_]+}} [[FIRST]]
// CHECK: [[SECOND:%[A-Za-z0-9_]+]] : [[BUFFER]] = OpFieldExtract [[BUFFERS]] [[STR_SECOND]]
// CHECK: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpCall [[SECOND_DISPATCHER:%[A-Za-z0-9_]+]] {{%[A-Za-z0-9_]+}} [[SECOND]]

// CHECK: Section("EMBD")
// CHECK-DAG: [[FIRST_DISPATCHER]] : {{%[A-Za-z0-9_]+}} = OpMakeDispatcher Compute {{%[A-Za-z0-9_]+}}
// CHECK-DAG: [[SECOND_DISPATCHER]] : {{%[A-Za-z0-9_]+}} = OpMakeDispatcher Compute {{%[A-Za-z0-9_]+}}
