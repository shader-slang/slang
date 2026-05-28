// Regression test: ConstantBuffer<T>.Handle with a non-struct element type and
// spvDescriptorHeapEXT should compile by wrapping T in a SPIR-V block struct.
// Tests vector, scalar, matrix, and array element types.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main1 -stage compute -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main2 -stage compute -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main3 -stage compute -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main4 -stage compute -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=SHARED): -target spirv-asm -entry mainShared -stage compute -capability spvDescriptorHeapEXT

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

// The synthesized wrapper must carry Block decoration (required for Uniform storage class).
// CHECK: OpDecorate %{{(cbuffer_.*_t|_Array_std140_float4)}} Block

// The synthesized wrapper must carry member layout decoration.
// CHECK: OpMemberDecorate %{{(cbuffer_.*_t|_Array_std140_float4)}} 0 Offset 0

// The pointer to the uniform block must be in Uniform storage class.
// CHECK: OpTypePointer Uniform

// The descriptor-heap access must produce a buffer pointer.
// CHECK: OpBufferPointerEXT

// Access to the wrapped element must use Uniform storage class, not Function.
// CHECK-NOT: OpAccessChain %_ptr_Function
// CHECK: OpAccessChain %_ptr_Uniform

// SHARED-COUNT-1: OpDecorate %cbuffer_float_t Block
// SHARED: OpMemberDecorate %cbuffer_float_t 0 Offset 0
// SHARED: OpBufferPointerEXT %_ptr_Uniform_cbuffer_float_t
// SHARED: OpAccessChain %_ptr_Uniform_float
// SHARED: OpBufferPointerEXT %_ptr_Uniform_cbuffer_float_t
// SHARED: OpAccessChain %_ptr_Uniform_float
