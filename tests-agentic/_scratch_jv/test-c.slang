//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
RWStructuredBuffer<int> outputBuffer;
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = 42;
    //CHECK: 42
}
