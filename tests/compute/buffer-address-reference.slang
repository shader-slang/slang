// Test for buffer address reference feature in TEST_INPUT
//
// This test verifies that buffer addresses can be referenced in other buffers
// using the syntax: data = [... bufferName ...]
// where bufferName is the name of a previously defined buffer.
//
// The referenced buffer's device address (64-bit) is patched into the
// data array at the reference location.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -cpu
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

// First buffer - its address will be referenced
//TEST_INPUT: set dataBuffer = ubuffer(data=[42.0 100.0], stride=4)
RWStructuredBuffer<float> dataBuffer;

// Second buffer - contains a pointer to dataBuffer
// The data layout: [ptr_low ptr_high] where ptr is the 64-bit address of dataBuffer
//TEST_INPUT: set ptrBuffer = ubuffer(data=[dataBuffer], stride=8)
RWStructuredBuffer<uint64_t> ptrBuffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Read the address stored in ptrBuffer
    uint64_t addr = ptrBuffer[0];
    
    // Verify the address is non-zero (valid pointer)
    outputBuffer[0] = (addr != 0) ? 1.0 : 0.0;  // CHECK: 1.0
    
    // Cast to pointer and read through it
    float* ptr = (float*)addr;
    outputBuffer[1] = ptr[0];  // CHECK: 42.0
}
