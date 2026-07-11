//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// The render-test host stores initialization data in uint32_t words. This 142-byte buffer exceeds
// List's initial capacity and checks that the final partial word is present when the RHI copies it.
//TEST_INPUT:ubuffer(data=[0], count=71, stride=2):out,name=outputBuffer
RWStructuredBuffer<uint16_t> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: type: uint16_t
    outputBuffer[0] = 1;
    // CHECK-NEXT: 1
    outputBuffer[1] = 2;
    // CHECK-NEXT: 2
    outputBuffer[2] = 3;
    // CHECK-NEXT: 3
}
