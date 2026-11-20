// byte-address-buffer.slang

//TEST:COMPARE_COMPUTE(targets=vk+cuda+cpu):
//TEST:COMPARE_COMPUTE(targets=dx12):-profile cs_6_0

//TEST_INPUT:ubuffer(data=[0 1 2 3]):name=inputBuffer
RWByteAddressBuffer inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0]):out,name=outputBuffer
RWByteAddressBuffer outputBuffer;

void testInt64(uint val)
{
    // Load a 32-bit value from the input buffer
    uint tmp = inputBuffer.Load(uint(val * 4));

    // Cast to uint64_t
    uint64_t tmp64 = uint64_t(tmp) + 1;

    // Store the result back as uint64_t
    outputBuffer.Store(uint(val * 8), tmp64);
}

[numthreads(4, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    testInt64(tid);
}
