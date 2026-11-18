//TEST:COMPARE_COMPUTE(targets=all):


//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);

    float a;

    switch (idx)
    {
        default:
        case 0:     a = 1.#INF; break;
        case 1:     a = 2.4#INFf; break;
        case 2:     a = float(234.5#INFl); break;
        case 3:     a = -1.#INF; break;
    }

    outputBuffer[idx] = a;
}
