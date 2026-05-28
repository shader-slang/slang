// Regression test: arrays of ConstantBuffer<T> with a non-struct element type
// should preserve the outer array while wrapping T in a SPIR-V block struct.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -capability spvDescriptorHeapEXT

[[vk::binding(0, 0)]]
ConstantBuffer<float> inputs[4];

RWStructuredBuffer<float> output;

[shader("compute")]
[NumThreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint index = tid.x & 3;
    output[0] = inputs[index];
}

// CHECK: OpName %[[CB:cbuffer_float_t]] "cbuffer_float_t"
// CHECK: OpDecorate %[[CB]] Block
// CHECK: OpMemberDecorate %[[CB]] 0 Offset 0
// CHECK: %[[ARRAY:_arr_cbuffer_float_t_int_4]] = OpTypeArray %[[CB]] %int_4
// CHECK: %[[ARRAY_PTR:_ptr_Uniform__arr_cbuffer_float_t_int_4]] = OpTypePointer Uniform %[[ARRAY]]
// CHECK: %[[CB_PTR:_ptr_Uniform_cbuffer_float_t]] = OpTypePointer Uniform %[[CB]]
// CHECK: OpAccessChain %[[CB_PTR]] %inputs
// CHECK: OpAccessChain %_ptr_Uniform_float
