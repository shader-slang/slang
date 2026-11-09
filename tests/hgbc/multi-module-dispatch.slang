//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -g1

import multi_module_payload;

[shader("compute")]
[numthreads(1, 1, 1)]
void fill(Payload payload)
{
    payload.output[0] = payloadValue();
}

[host]
public void main(RWStructuredBuffer<uint> output)
{
    let payload = Payload(output);
    dispatch(fill, uint3(1, 1, 1))(payload);
}

// CHECK: Section("LITR")
// CHECK-DAG: [[STR_PAYLOAD_MODULE:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit {{[0-9]+}} "multi_module_payload"
// CHECK-DAG: [[STR_FILL:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpStringLit 4 "fill"
// CHECK-DAG: [[PAYLOAD_BLOB:%[A-Za-z0-9_]+]] = OpBinaryLit {{[0-9]+}} { ... }
// CHECK-DAG: [[GRAPH_BLOB:%[A-Za-z0-9_]+]] = OpBinaryLit {{[0-9]+}} { ... }

// The imported source is referenced by both its own IR module and the importing module. Both
// producers must use the same stable path identity so IR deduplication leaves one debug source per
// physical file. Otherwise a basename breakpoint matches two path keys and is reported as
// ambiguous.
// CHECK: Section("META")
// CHECK-COUNT-2: OpDebugSource
// CHECK-NOT: OpDebugSource

// CHECK: Section("FUNC")
// CHECK: [[DISPATCH_SIZE:%[A-Za-z0-9_]+]] : {{%[A-Za-z0-9_]+}} = OpMakeVector
// CHECK: OpCall [[DISPATCHER:%[A-Za-z0-9_]+]] [[DISPATCH_SIZE]]

// CHECK: Section("EMBD")
// CHECK-DAG: [[PAYLOAD_MODULE:%[A-Za-z0-9_]+]] = OpSlangModule [[STR_PAYLOAD_MODULE]] [[PAYLOAD_BLOB]]
// CHECK-DAG: [[GRAPH_MODULE:%[A-Za-z0-9_]+]] = OpSlangModule {{%[A-Za-z0-9_]+}} [[GRAPH_BLOB]] [[PAYLOAD_MODULE]]
// CHECK-DAG: [[FILL_ENTRY:%[A-Za-z0-9_]+]] = OpSlangEntryPoint [[GRAPH_MODULE]] [[STR_FILL]]
// CHECK-DAG: [[DISPATCHER]] : {{%[A-Za-z0-9_]+}} = OpMakeDispatcher Compute [[FILL_ENTRY]]
