//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// CHECK:      123
// CHECK-NEXT: 4567
// CHECK-NEXT: 0
// CHECK-NEXT: 0

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint a = outputBuffer[0];
    uint b = outputBuffer[1];
    uint c = outputBuffer[2];
    uint d = outputBuffer[3];

    outputBuffer[0] = a;
    outputBuffer[1] = b;
    outputBuffer[2] = c;
    outputBuffer[3] = d;
}
