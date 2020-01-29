//TEST(compute):COMPARE_COMPUTE:-cuda
//TEST(compute):COMPARE_COMPUTE:-cpu
//TEST(compute):COMPARE_COMPUTE:
//TEST(compute,vulcan):COMPARE_COMPUTE:-vk

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer : register(u0);

int quantize(double value)
{
    return int(value * 256);
}

int quantize(float value)
{
    return int(value * 256);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float values[] = { -9, 9, -3, 3 };

	int tid = int(dispatchThreadID.x);
    float value = values[tid];
    
    outputBuffer[tid * 4] = quantize(sin(value));
    outputBuffer[tid * 4 + 1] = quantize(cos(value));
    
    outputBuffer[tid * 4 + 2] = quantize(sin(double(value)));
    outputBuffer[tid * 4 + 3] = quantize(cos(double(value)));
}