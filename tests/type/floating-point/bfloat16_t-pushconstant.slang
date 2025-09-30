//TEST:SIMPLE(filecheck=SPIRV): -target spirv -stage compute -entry main -O0

struct PushConstants
{
    //SPIRV: OpDecorate %_arr_bfloat16_int_32 ArrayStride 2
    bfloat16_t value[32];
};

[[vk::push_constant]]
PushConstants pushConstants;

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    const int idx = dispatchThreadID.x;
    //SPIRV: [[PCO:%.*]] = OpAccessChain %_ptr_PushConstant__Array_std430_bfloat1632 %pushConstants %int_0
    //SPIRV: [[PCO_BF16ARR:%.*]] = OpAccessChain %_ptr_PushConstant__arr_bfloat16_int_32 [[PCO]] %int_0
    //SPIRV: [[PCO_BF16ADDR:%.*]] = OpAccessChain %_ptr_PushConstant_bfloat16 [[PCO_BF16ARR]] %int_0
    //SPIRV: [[BF16DATA:%.*]] = OpLoad %bfloat16 [[PCO_BF16ADDR]]
    //SPIRV: OpFConvert %float [[BF16DATA]]
    let val = float(pushConstants.value[0]);
    outputBuffer[idx] = val;
}
