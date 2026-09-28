// A `ConstantBuffer<T>` and a `RWStructuredBuffer<T>` fetched from the same descriptor
// heap must keep distinct SPIR-V descriptor kinds: the constant buffer is a uniform-buffer
// descriptor (`OpTypeBufferEXT Uniform`) whose data pointer is an untyped Uniform pointer
// addressed with `OpUntypedAccessChainKHR`, while the structured buffer is a storage-buffer
// descriptor. Emitting the constant buffer as a StorageBuffer descriptor (the #11647
// regression) makes the driver read a storage-buffer descriptor from a heap slot the
// application filled with a uniform-buffer descriptor. See #12226.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -capability spvDescriptorHeapEXT

struct UniformData
{
    float value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    ConstantBuffer<UniformData> uniforms = ResourceDescriptorHeap[0];
    RWStructuredBuffer<float> storageBuffer = ResourceDescriptorHeap[123];
    storageBuffer[dispatchThreadID.x] = uniforms.value;
}

// The constant buffer's heap descriptor is a uniform-buffer descriptor, its data pointer is
// an untyped Uniform pointer, and its member is addressed with an untyped access chain whose
// Base Type is the block struct.
// CHECK-DAG: OpTypeBufferEXT Uniform
// CHECK-DAG: %[[UNIFORM_PTR:_ptr_Uniform]] = OpTypeUntypedPointerKHR Uniform{{$}}
// CHECK-DAG: OpBufferPointerEXT %[[UNIFORM_PTR]]
// CHECK-DAG: OpUntypedAccessChainKHR %[[UNIFORM_PTR]] %{{[A-Za-z0-9_]*}}UniformData

// The structured buffer keeps a storage-buffer descriptor with a typed StorageBuffer pointer,
// proving the two descriptor kinds stay distinct.
// CHECK-DAG: OpTypeBufferEXT StorageBuffer
// CHECK-DAG: OpTypePointer StorageBuffer %RWStructuredBuffer
