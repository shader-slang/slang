//TEST:COMPARE_COMPUTE(targets=all): -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

float quantize(float value)
{
    return int(value * 256) * 1.0f / 256.0f;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float values[] = { -9, 9, -3, 3 };

	int tid = int(dispatchThreadID.x);
    float value = values[tid];

    outputBuffer[tid * 2] = quantize(sin(value));
    outputBuffer[tid * 2 + 1] = quantize(cos(value));
}