//TEST:SIMPLE(filecheck=CHECK): -target hgbc

[host]
public StructuredBuffer<uint> allocateIndexBuffer(uint indexCount)
{
    return StructuredBuffer<uint>::alloc(indexCount);
}

// CHECK-DAG: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType
// CHECK-DAG: [[BUFFER:%[A-Za-z0-9_]+]] = OpStructuredBufferType [[UINT]]
// CHECK: [[RESULT:%[A-Za-z0-9_]+]] : [[BUFFER]] = OpAllocateResource Buffer Buffer Undefined DeviceLocal UnorderedAccess
// CHECK-SAME: [[UINT]] {{%[A-Za-z0-9_]+}} {{%[A-Za-z0-9_]+}} {{%[A-Za-z0-9_]+}} 1 1 0 [[LAYOUT:%[A-Za-z0-9_]+]] ShaderResource
// CHECK: OpReturn [[RESULT]]
// CHECK: Section("EMBD")
// CHECK: [[LAYOUT]] = OpTypeLayout [[UINT]] DeviceMemory StructuredBufferElement {{%[A-Za-z0-9_]+}} 0
