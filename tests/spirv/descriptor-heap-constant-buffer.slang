// Regression test for #11037: segmentation fault (assertion failure) when
// ConstantBuffer<T>.Handle is used with spvDescriptorHeapEXT.
//
// The SPIRV legalize pass was not lowering ConstantBuffer<T> to a Uniform pointer,
// so the SPIRV emitter's getDescriptorHeapBufferStorageClass hit an assertion.

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

// The struct must carry Block decoration (required for Uniform storage class).
// CHECK: OpName %[[CB:Uniforms_std140]] "Uniforms_std140"
// CHECK-NOT: %_ptr_Function_[[CB]]
// CHECK: OpDecorate %[[CB]] Block

// The pointer to the uniform block must be in Uniform storage class.
// CHECK-NOT: %_ptr_Function_[[CB]]
// CHECK: %[[CB_PTR:_ptr_Uniform_Uniforms_std140]] = OpTypePointer Uniform %[[CB]]

// The descriptor-heap access must produce a buffer pointer.
// CHECK-NOT: %_ptr_Function_[[CB]]
// CHECK: OpBufferPointerEXT %[[CB_PTR]]

// Field access must use Uniform storage class — not Function.
// CHECK: OpAccessChain %_ptr_Uniform_uint
// CHECK-NOT: %_ptr_Function_[[CB]]

// SHARED_STRUCT: OpName %[[CB:SharedUniforms_std140]] "SharedUniforms_std140"
// SHARED_STRUCT: OpName %[[SB:SharedUniforms_std430]] "SharedUniforms_std430"
// SHARED_STRUCT-NOT: OpDecorate %[[SB]] Block
// SHARED_STRUCT: OpDecorate %[[CB]] Block
// SHARED_STRUCT: %[[CB_PTR:_ptr_Uniform_SharedUniforms_std140]] = OpTypePointer Uniform %[[CB]]
// SHARED_STRUCT: %[[SB_PTR:_ptr_StorageBuffer_StructuredBuffer]] = OpTypePointer StorageBuffer %StructuredBuffer
// SHARED_STRUCT: OpBufferPointerEXT %[[CB_PTR]]
// SHARED_STRUCT: OpBufferPointerEXT %[[SB_PTR]]
