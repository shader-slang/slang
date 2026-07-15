//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// List's initial capacity is 16 uint32_t words (64 bytes). This 142-byte buffer therefore forces
// the old code to allocate only 35 words (140 bytes), so ASan detects the RHI's two-byte over-read;
// the shader checks alone cannot observe this host-side allocation bug.
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
