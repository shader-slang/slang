
//TEST:COMPARE_COMPUTE(targets=vk+cuda+cpu, filecheck-buffer=CHECK):-output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    int c = 1;
    let lambda = (int x, int y) => x + y + c;
    let result = lambda(3, 4);
    outputBuffer[0] = result; // Expecting outputBuffer[0] to be 8
    // CHECK: 8
}