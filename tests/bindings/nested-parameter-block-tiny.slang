//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -d3d12 -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -metal -shaderobj -output-using-type -render-features argument-buffer-tier-2

//TEST_INPUT: set pb = 1.0
ParameterBlock<ParameterBlock<float>> pb;

//TEST_INPUT: ubuffer(data=[0], stride=4, count=1):out,name outBuffer
RWStructuredBuffer<float> outBuffer;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 sv_dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK: 1.000000
    outBuffer[0] = pb;
}
