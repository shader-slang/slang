// Float16 not supported on CPU currently
// Doesn't work on FXC

//TEST:COMPARE_COMPUTE(targets=dx12):-profile sm_6_2 -render-feature half -output-using-type
//TEST:COMPARE_COMPUTE(targets=vk):-render-feature int16,half -output-using-type
//TEST:COMPARE_COMPUTE(targets=cuda):-render-features half -output-using-type
//TEST:COMPARE_COMPUTE(targets=llvm):-output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0  0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);

    // 1
    float16_t m = asfloat16(uint16_t(0x3c00));

    float16_t v = asfloat16(uint16_t(0x3c00 + (idx << 8))) + m;
    float f = v;

    outputBuffer[idx] = f;
}
