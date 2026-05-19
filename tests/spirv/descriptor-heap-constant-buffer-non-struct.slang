// Regression test: ConstantBuffer<T>.Handle with a non-struct element type and
// spvDescriptorHeapEXT should compile by wrapping T in a SPIR-V block struct.
// Tests vector, scalar, and matrix element types.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main1 -stage compute -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main2 -stage compute -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main3 -stage compute -capability spvDescriptorHeapEXT

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

// The synthesized wrapper must carry Block decoration (required for Uniform storage class).
// CHECK: OpDecorate {{.*}} Block

// The pointer to the uniform block must be in Uniform storage class.
// CHECK: OpTypePointer Uniform

// The descriptor-heap access must produce a buffer pointer.
// CHECK: OpBufferPointerEXT

// Access to the wrapped element must use Uniform storage class, not Function.
// CHECK-NOT: OpAccessChain %_ptr_Function
// CHECK: OpAccessChain %_ptr_Uniform
