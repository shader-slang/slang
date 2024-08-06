//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF): -vk -compute -shaderobj

import synth_constructor_private_internal_helper;

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test val1 = getTest();
    Test val2 = Test::getTest();
    outputBuffer[0] = (true
        && val1.getVal2() == 1
        && val1.getNested1().getVal1() == 0
        && val1.getNested1().getVal2() == 0
        && val1.getNested2().getVal1() == 0
        && val1.getNested2().getVal2() == 0

        && val2.getVal1() == 1
        && val2.getVal2() == 2
        && val2.getNested1().getVal2() == 3
        && val2.getNested2().getVal2() == 4
        ) ? 1 : 0;

// BUF: 1
}