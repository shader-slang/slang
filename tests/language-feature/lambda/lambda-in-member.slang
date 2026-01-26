
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

struct Composite<int base>
{
    static int member = (()=>1+base)();
}


[numthreads(1,1,1)]
void computeMain()
{
    outputBuffer[0] = Composite<3>.member;
    // CHECK: 4
}