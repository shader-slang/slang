//TEST:COMPARE_COMPUTE(targets=all-wgpu):-output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(in uint i : SV_GroupIndex)
{
    outputBuffer[i] = int(clamp(double(i), 1.0, 2.0) * 100);
}
