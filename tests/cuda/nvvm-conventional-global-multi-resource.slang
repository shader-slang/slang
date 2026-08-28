// Exercise multiple executable fields in the conventional CUDA global-parameter block.

//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -output-using-type -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -output-using-type -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
//TEST_INPUT:ubuffer(data=[10 20 30 40], stride=4):name=secondBuffer
RWStructuredBuffer<int> outputBuffer;
RWStructuredBuffer<int> secondBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = int(dispatchThreadID.x);
    outputBuffer[index] = secondBuffer[index] + 1;
}
