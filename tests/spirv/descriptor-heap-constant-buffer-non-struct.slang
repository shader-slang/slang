// Regression test: ConstantBuffer<T>.Handle with a non-struct element type and
// spvDescriptorHeapEXT should compile by wrapping T in a SPIR-V block struct.
// Tests vector, scalar, matrix, and array element types.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main1 -stage compute -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main2 -stage compute -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main3 -stage compute -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=CHECK_ARRAY): -target spirv-asm -entry main4 -stage compute -capability spvDescriptorHeapEXT
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

// main1/main2/main3 — scalar, vector, and matrix elements are wrapped into a block struct and
// fetched as a uniform-buffer descriptor whose data pointer is an untyped Uniform pointer, with
// the member reached through an untyped access chain (since #12226).
//
// The synthesized wrapper must carry Block + member layout decoration.
// CHECK: OpName %[[CB:cbuffer_[A-Za-z0-9_]+_t]]
// CHECK: OpDecorate %[[CB]] Block
// CHECK: OpMemberDecorate %[[CB]] 0 Offset 0
// The descriptor-heap load produces an untyped Uniform buffer pointer, and the element is
// reached with an untyped access chain naming the wrapper struct — never a typed StorageBuffer
// constant-buffer pointer.
// CHECK: OpTypeBufferEXT Uniform
// CHECK: %[[UNIFORM_PTR:_ptr_Uniform]] = OpTypeUntypedPointerKHR Uniform{{$}}
// CHECK: OpBufferPointerEXT %[[UNIFORM_PTR]]
// CHECK: OpUntypedAccessChainKHR %[[UNIFORM_PTR]] %[[CB]]
// CHECK-NOT: OpTypePointer StorageBuffer %[[CB]]

// main4 — ConstantBuffer<float[4]> via heap: the array element is wrapped into a block struct
// and addressed through the untyped Uniform pointer. The nested array carries an ordinary
// array-type ArrayStride (16 under std140); no pointer-type ArrayStride is needed.
// CHECK_ARRAY: OpName %[[CBA:_Array_std140_float4]] "_Array_std140_float4"
// CHECK_ARRAY-DAG: OpDecorate %[[CBA]] Block
// CHECK_ARRAY-DAG: OpDecorate %_arr_float_int_4 ArrayStride 16
// CHECK_ARRAY: %[[UNIFORM_PTR:_ptr_Uniform]] = OpTypeUntypedPointerKHR Uniform{{$}}
// CHECK_ARRAY: OpBufferPointerEXT %[[UNIFORM_PTR]]
// CHECK_ARRAY: OpUntypedAccessChainKHR %[[UNIFORM_PTR]] %[[CBA]]

// The wrapper type is created once and both value users are rewritten (no duplicate
// `cbuffer_float_t_` name), and both fetches share the untyped Uniform pointer.
// SHARED: OpName %[[CB:cbuffer_float_t]] "cbuffer_float_t"
// SHARED-NOT: cbuffer_float_t_
// SHARED: OpDecorate %[[CB]] Block
// SHARED: OpMemberDecorate %[[CB]] 0 Offset 0
// SHARED: %[[UNIFORM_PTR:_ptr_Uniform]] = OpTypeUntypedPointerKHR Uniform{{$}}
// SHARED: OpBufferPointerEXT %[[UNIFORM_PTR]]
// SHARED: OpUntypedAccessChainKHR %[[UNIFORM_PTR]] %[[CB]]
// SHARED: OpBufferPointerEXT %[[UNIFORM_PTR]]
// SHARED: OpUntypedAccessChainKHR %[[UNIFORM_PTR]] %[[CB]]
// SHARED-NOT: cbuffer_float_t_

// HANDLE_ARRAY: OpName %[[CB:cbuffer_float_t]] "cbuffer_float_t"
// HANDLE_ARRAY: OpDecorate %[[CB]] Block
// HANDLE_ARRAY: OpMemberDecorate %[[CB]] 0 Offset 0
// HANDLE_ARRAY-DAG: OpTypeArray %v2uint %int_4
// HANDLE_ARRAY-DAG: %[[UNIFORM_PTR:_ptr_Uniform]] = OpTypeUntypedPointerKHR Uniform{{$}}
// HANDLE_ARRAY: OpAccessChain %_ptr_PushConstant_{{(_arr_v2uint_int_4|v2uint)}}
// HANDLE_ARRAY: OpBufferPointerEXT %[[UNIFORM_PTR]]
// HANDLE_ARRAY: OpUntypedAccessChainKHR %[[UNIFORM_PTR]] %[[CB]]
