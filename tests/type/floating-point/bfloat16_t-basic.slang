//TEST:SIMPLE(filecheck=SPIRV): -target spirv -stage compute -entry main -O0

//SPIRV: OpCapability BFloat16TypeKHR

RWStructuredBuffer<bfloat16_t> outputBuffer0;
RWStructuredBuffer<half> outputBuffer1;

//SPIRV: %bfloat16 = OpTypeFloat 16 BFloat16KHR
//SPIRV: %bfloat16_0x1p_0 = OpConstant %bfloat16 0x1p+0
static const let bf16Val = 1.bf;

//SPIRV: %half = OpTypeFloat 16
//SPIRV: %half_0x1p_0 = OpConstant %half 0x1p+0

[numthreads(1, 1, 1)]
void main(int3 dispatchThreadID : SV_DispatchThreadID)
{
    const int idx = dispatchThreadID.x;
    //SPIRV: OpStore {{.*}} %bfloat16_0x1p_0
    outputBuffer0[idx] = bf16Val;
    //SPIRV: OpStore {{.*}} %half_0x1p_0
    outputBuffer1[idx] = half(bf16Val);
}
