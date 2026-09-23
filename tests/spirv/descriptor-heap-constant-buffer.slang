// Regression test for #11037: segmentation fault (assertion failure) when
// ConstantBuffer<T>.Handle is used with spvDescriptorHeapEXT.
//
// The SPIRV legalize pass was not lowering ConstantBuffer<T> to a buffer pointer,
// so the SPIRV emitter's getDescriptorHeapBufferStorageClass hit an assertion.
// Since #12226 the descriptor-heap constant buffer is fetched as a uniform-buffer
// descriptor (`OpTypeBufferEXT Uniform`) whose data pointer is an untyped Uniform pointer,
// while structured buffers keep a storage-buffer descriptor.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=SHARED_STRUCT): -target spirv-asm -entry mainSharedStruct -stage compute -capability spvDescriptorHeapEXT

struct Uniforms {
    uint x;
}

struct SharedUniforms {
    uint v;
}

[shader("compute")]
[NumThreads(1, 1, 1)]
void main(
    uniform DescriptorHandle<ConstantBuffer<Uniforms>> input,
    uniform DescriptorHandle<RWStructuredBuffer<uint>> output)
{
    output[0] = getDescriptorFromHandle(input).x;
}

[shader("compute")]
[NumThreads(1, 1, 1)]
void mainSharedStruct(
    uniform DescriptorHandle<ConstantBuffer<SharedUniforms>> input,
    uniform DescriptorHandle<StructuredBuffer<SharedUniforms>> values,
    uniform DescriptorHandle<RWStructuredBuffer<uint>> output)
{
    output[0] = getDescriptorFromHandle(input).v + getDescriptorFromHandle(values)[0].v;
}

// The constant-buffer element struct carries Block, its heap descriptor is a uniform-buffer
// descriptor, and its member is reached with an untyped Uniform access chain — never a typed
// StorageBuffer constant-buffer pointer.
// CHECK: OpName %[[CB:Uniforms_std140]] "Uniforms_std140"
// CHECK: OpDecorate %[[CB]] Block
// CHECK: OpTypeBufferEXT Uniform
// CHECK: %[[UNIFORM_PTR:_ptr_Uniform]] = OpTypeUntypedPointerKHR Uniform{{$}}
// CHECK: OpBufferPointerEXT %[[UNIFORM_PTR]]
// CHECK: OpUntypedAccessChainKHR %[[UNIFORM_PTR]] %[[CB]]
// CHECK-NOT: OpTypePointer StorageBuffer %[[CB]]

// The constant buffer is a uniform-buffer descriptor + untyped Uniform pointer, while the
// structured buffer stays a storage-buffer descriptor with a typed StorageBuffer pointer —
// distinct descriptor kinds in one module.
// SHARED_STRUCT: OpName %[[CB:SharedUniforms_std140]] "SharedUniforms_std140"
// SHARED_STRUCT: OpName %[[SB:SharedUniforms_std430]] "SharedUniforms_std430"
// SHARED_STRUCT-NOT: OpDecorate %[[SB]] Block
// SHARED_STRUCT: OpDecorate %[[CB]] Block
// SHARED_STRUCT: %[[UNIFORM_PTR:_ptr_Uniform]] = OpTypeUntypedPointerKHR Uniform{{$}}
// SHARED_STRUCT: %[[SB_PTR:_ptr_StorageBuffer_StructuredBuffer]] = OpTypePointer StorageBuffer %StructuredBuffer
// SHARED_STRUCT: OpBufferPointerEXT %[[UNIFORM_PTR]]
// SHARED_STRUCT: OpUntypedAccessChainKHR %[[UNIFORM_PTR]] %[[CB]]
// SHARED_STRUCT: OpBufferPointerEXT %[[SB_PTR]]
