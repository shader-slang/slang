//TEST:SIMPLE(filecheck=CHECK): -target hgbc

// Atomic access is a shader-side property of this resource. Its HGBC allocation and layout use
// the stored `uint` ABI type, while the serialized Slang module retains `Atomic<uint>`.
[host]
public RWStructuredBuffer<Atomic<uint>> allocateAtomicCounters(uint count)
{
    return RWStructuredBuffer<Atomic<uint>>::alloc(count);
}

// CHECK-DAG: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType
// CHECK-DAG: [[BUFFER:%[A-Za-z0-9_]+]] = OpRWStructuredBufferType [[UINT]]
// CHECK: [[RESULT:%[A-Za-z0-9_]+]] : [[BUFFER]] = OpAllocateResource
// CHECK-SAME: Buffer Buffer Undefined DeviceLocal UnorderedAccess [[UINT]]
// CHECK-SAME: {{%[A-Za-z0-9_]+}} {{%[A-Za-z0-9_]+}} {{%[A-Za-z0-9_]+}} 1 1 0
// CHECK-SAME: [[LAYOUT:%[A-Za-z0-9_]+]] ShaderResource
// CHECK: OpReturn [[RESULT]]
// CHECK: Section("EMBD")
// CHECK: [[LAYOUT]] = OpTypeLayout [[UINT]] DeviceMemory StructuredBufferElement
// CHECK-SAME: {{%[A-Za-z0-9_]+}} 0
