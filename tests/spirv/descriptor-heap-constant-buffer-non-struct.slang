// Regression test: ConstantBuffer<T>.Handle with a non-struct element type and
// spvDescriptorHeapEXT should compile by wrapping T in a SPIR-V block struct.
// Tests vector, scalar, matrix, and array element types.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main1 -stage compute -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main2 -stage compute -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main3 -stage compute -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main4 -stage compute -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=SHARED): -target spirv-asm -entry mainShared -stage compute -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=HANDLE_ARRAY): -target spirv-asm -entry mainHandleArray -stage compute -capability spvDescriptorHeapEXT

[shader("compute")]
[NumThreads(1, 1, 1)]
void main1(
    uniform DescriptorHandle<ConstantBuffer<float2>> input,
    uniform DescriptorHandle<RWStructuredBuffer<float2>> output)
{
    output[0] = getDescriptorFromHandle(input);
}

[shader("compute")]
[NumThreads(1, 1, 1)]
void main2(
    uniform DescriptorHandle<ConstantBuffer<float>> input,
    uniform DescriptorHandle<RWStructuredBuffer<float>> output)
{
    output[0] = getDescriptorFromHandle(input);
}

[shader("compute")]
[NumThreads(1, 1, 1)]
void main3(
    uniform DescriptorHandle<ConstantBuffer<float4x4>> input,
    uniform DescriptorHandle<RWStructuredBuffer<float4x4>> output)
{
    output[0] = getDescriptorFromHandle(input);
}

[shader("compute")]
[NumThreads(1, 1, 1)]
void main4(
    uniform DescriptorHandle<ConstantBuffer<float[4]>> input,
    uniform DescriptorHandle<RWStructuredBuffer<float>> output)
{
    output[0] = getDescriptorFromHandle(input)[2];
}

// Use the same ConstantBuffer<T> type twice so the legalization pass has to
// wrap the type once and rewrite both value users.
[shader("compute")]
[NumThreads(1, 1, 1)]
void mainShared(
    uniform DescriptorHandle<ConstantBuffer<float>> left,
    uniform DescriptorHandle<ConstantBuffer<float>> right,
    uniform DescriptorHandle<RWStructuredBuffer<float>> output)
{
    output[0] = getDescriptorFromHandle(left) + getDescriptorFromHandle(right);
}

[shader("compute")]
[NumThreads(1, 1, 1)]
void mainHandleArray(
    uniform ConstantBuffer<float>.Handle inputs[4],
    uniform DescriptorHandle<RWStructuredBuffer<float>> output,
    uint3 tid : SV_DispatchThreadID)
{
    uint index = tid.x & 3;
    output[0] = getDescriptorFromHandle(inputs[index]);
}

// The synthesized wrapper must carry Block decoration (required for Uniform storage class).
// CHECK: OpName %[[CB:(cbuffer_[A-Za-z0-9_]+_t|_Array_std140_float4)]]
// CHECK-NOT: %_ptr_Function_[[CB]]
// CHECK: OpDecorate %[[CB]] Block

// The synthesized wrapper must carry member layout decoration.
// CHECK: OpMemberDecorate %[[CB]] 0 Offset 0

// The pointer to the uniform block must be in Uniform storage class.
// CHECK-NOT: %_ptr_Function_[[CB]]
// CHECK: %[[CB_PTR:_ptr_Uniform_[A-Za-z0-9_]+]] = OpTypePointer Uniform %[[CB]]

// The descriptor-heap access must produce a buffer pointer.
// CHECK-NOT: %_ptr_Function_[[CB]]
// CHECK: OpBufferPointerEXT %[[CB_PTR]]

// Access to the wrapped element must use Uniform storage class, not Function.
// CHECK: OpAccessChain %_ptr_Uniform
// CHECK-NOT: %_ptr_Function_[[CB]]

// SHARED: OpName %[[CB:cbuffer_float_t]] "cbuffer_float_t"
// SHARED-NOT: cbuffer_float_t_
// SHARED: OpDecorate %[[CB]] Block
// SHARED-NOT: cbuffer_float_t_
// SHARED: OpMemberDecorate %[[CB]] 0 Offset 0
// SHARED: %[[CB_PTR:_ptr_Uniform_cbuffer_float_t]] = OpTypePointer Uniform %[[CB]]
// SHARED-NOT: cbuffer_float_t_
// SHARED: OpBufferPointerEXT %[[CB_PTR]]
// SHARED: OpAccessChain %_ptr_Uniform_float
// SHARED: OpBufferPointerEXT %[[CB_PTR]]
// SHARED: OpAccessChain %_ptr_Uniform_float
// SHARED-NOT: cbuffer_float_t_

// HANDLE_ARRAY: OpName %[[CB:cbuffer_float_t]] "cbuffer_float_t"
// HANDLE_ARRAY: OpDecorate %[[CB]] Block
// HANDLE_ARRAY: OpMemberDecorate %[[CB]] 0 Offset 0
// HANDLE_ARRAY: %[[CB_PTR:_ptr_Uniform_cbuffer_float_t]] = OpTypePointer Uniform %[[CB]]
// HANDLE_ARRAY: OpAccessChain %_ptr_PushConstant__arr_v2uint_int_4
// HANDLE_ARRAY: OpBufferPointerEXT %[[CB_PTR]]
// HANDLE_ARRAY: OpAccessChain %_ptr_Uniform_float
