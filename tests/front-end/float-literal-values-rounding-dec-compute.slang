// Test that decimal float literal parsing produces correctly rounded
// values for float/half literals. Rounding mode is the standard IEEE 754
// round to nearest, ties to even.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;


//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -metal -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu -compute -shaderobj
void computeMain()
{
    uint i = 0;

    ////// regular positive floats -- round to nearest, ties to even //////

    float f;

    outputBuffer[i++] = bit_cast<uint32_t>(8388608.4999999999999999999f);
    //CHECK: 4B000000

    outputBuffer[i++] = bit_cast<uint32_t>(8388608.5f); // even is towards zero
    //CHECK: 4B000000

    outputBuffer[i++] = bit_cast<uint32_t>(8388608.5000000000000000001f);
    //CHECK: 4B000001

    outputBuffer[i++] = bit_cast<uint32_t>(8388609.4999999999999999999f);
    //CHECK: 4B000001

    outputBuffer[i++] = bit_cast<uint32_t>(8388609.5f); // even is away from zero
    //CHECK: 4B000002

    outputBuffer[i++] = bit_cast<uint32_t>(8388609.5000000000000000001f);
    //CHECK: 4B000002


    ////// regular positive half-floats -- round to nearest, ties to even //////

    half h;

    outputBuffer[i++] = uint32_t(bit_cast<uint16_t>(61440.hf));
    //CHECK: 7B80

    outputBuffer[i++] = uint32_t(bit_cast<uint16_t>(61456.hf)); // round to even = towards zero
    //CHECK: 7B80

    outputBuffer[i++] = uint32_t(bit_cast<uint16_t>(61456.0001hf));
    //CHECK: 7B81

    outputBuffer[i++] = uint32_t(bit_cast<uint16_t>(61472.hf));
    //CHECK: 7B81

    outputBuffer[i++] = uint32_t(bit_cast<uint16_t>(61488.hf)); // round to even = away from zero
    //CHECK: 7B82

    outputBuffer[i++] = uint32_t(bit_cast<uint16_t>(61504.hf));
    //CHECK: 7B82
}
