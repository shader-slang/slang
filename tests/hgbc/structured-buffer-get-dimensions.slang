//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[host]
public void main(RWStructuredBuffer<uint> buffer)
{
    uint count;
    uint stride;
    buffer.GetDimensions(count, stride);
    print(count);
    print(stride);
}

// CHECK: Section("LITR")
// CHECK-DAG: [[ZERO:%[A-Za-z0-9_]+]] : [[UINT:%[A-Za-z0-9_]+]] = OpIntLit 0
// CHECK-DAG: [[ONE:%[A-Za-z0-9_]+]] : [[UINT]] = OpIntLit 1

// CHECK: Section("META")
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[UINT]] = OpTypeSize 4 4 4

// CHECK: Section("TYPE")
// CHECK-DAG: [[UINT]] = OpUIntType
// CHECK-DAG: [[UINT2:%[A-Za-z0-9_]+]] = OpVectorType 2 [[UINT]]
// CHECK-DAG: [[BUFFER_TYPE:%[A-Za-z0-9_]+]] = OpRWStructuredBufferType [[UINT]]

// CHECK: Section("FUNC")
// CHECK-DAG: [[BUFFER:%[A-Za-z0-9_]+]] : [[BUFFER_TYPE]] = OpParam
// CHECK-DAG: [[DIMS:%[A-Za-z0-9_]+]] : [[UINT2]] = OpStructuredBufferGetDimensions [[BUFFER_TYPE]] [[BUFFER]]
// CHECK-DAG: [[COUNT:%[A-Za-z0-9_]+]] : [[UINT]] = OpExtractElement [[DIMS]] [[ZERO]]
// CHECK-DAG: [[STRIDE:%[A-Za-z0-9_]+]] : [[UINT]] = OpExtractElement [[DIMS]] [[ONE]]
// CHECK: OpPrint [[UINT]] [[COUNT]]
// CHECK: OpPrint [[UINT]] [[STRIDE]]
