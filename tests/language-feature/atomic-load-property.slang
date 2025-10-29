//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type

struct AtomicCounter
{
    Atomic<uint> sharedNumber;
    property uint value { [constref] get { return sharedNumber.load(); } }
}

//TEST_INPUT: set gBuffer = ubuffer(data=[10 0 0 0], stride=4)
RWStructuredBuffer<AtomicCounter> gBuffer;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    // CHECK: 10
    outputBuffer[0] = gBuffer[0].value;
}
