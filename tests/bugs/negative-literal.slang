//TEST:COMPARE_COMPUTE(targets=cpu):-compile-arg -O3
//TEST:COMPARE_COMPUTE(targets=all):

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float v = asfloat((int(dispatchThreadID.x) & -2147483648) | 1065353216);
	outputBuffer[dispatchThreadID.x] = v;
}
