//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF): -vk -compute -shaderobj

import synth_constructor_public_private_internal_helper;

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test val1 = { 1, { 2 } };
    Test val2 = makeInternal();
	outputBuffer[0] = (true
        && val1.val3 == 1
        && val1.nested1.val3 == 2

        && val2.getVal1() == 1
        && val2.val3 == 2
        && val2.nested1.getVal1() == 3
        && val2.nested1.val3 == 4
        ) ? 1 : 0;

// BUF: 1
}