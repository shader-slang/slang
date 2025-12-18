//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
func computeMain(tid : int : SV_DispatchThreadID) -> void
{
    // CHECK: 1
    outputBuffer[0] = tid + 1;
}