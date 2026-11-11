//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-d3d12 -output-using-type
// TODO: Atomics not supported on LLVM yet.
//DISABLE_TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -llvm

struct AtomicCounter
{
    Atomic<uint> sharedNumber;
    // Preserve the atomic's storage identity through the property receiver as well.
    property uint value { [__ref] get { return sharedNumber.load(); } }
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
