//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-mtl -compute -shaderobj -output-using-type

//TEST_INPUT: ubuffer(data=[0]):name=buffer
RWByteAddressBuffer buffer;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void computeMain( uint2 dispatchThreadID : SV_DispatchThreadID )
{
    buffer.InterlockedAddF32(0, 1.0f);
    outputBuffer[dispatchThreadID.x] = buffer.Load<float>(0);
}