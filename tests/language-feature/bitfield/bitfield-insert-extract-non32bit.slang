//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -output-using-type
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

//TEST_INPUT:set input = ubuffer(data=[0xC0A0], stride=4)
RWStructuredBuffer<uint32_t> input;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(
    uint3 tid : SV_DispatchThreadID)
{
    uint16_t a = uint16_t(input[0] & 0xFFFF);
    int16_t b = bit_cast<int16_t>(a);

    outputBuffer[0] = bitfieldExtract(a, 14, 2);
    outputBuffer[1] = bitfieldExtract(b, 13, 3);    // 0b110, with sign extend, it will be 0xFFFFFFFE => -2
    // BUF: 3
    // BUF: -2

    uint8_t c = uint8_t(input[0] & 0xFF);
    int8_t d = bit_cast<int8_t>(c);

    outputBuffer[2] = bitfieldExtract(c, 5, 3);     // b101 = 5
    outputBuffer[3] = bitfieldExtract(d, 5, 3);     // 0b101, with sign extend, it will be 0xFFFFFFFD => -3
    // BUF: 5
    // BUF: -3

    outputBuffer[4] = bitfieldInsert(a, 0x03U, 8, 2);   // 0xC3A0 = 50080 (unsigned)
    outputBuffer[5] = bitfieldInsert(b, 0x03,  8, 2);    // 0xC3A0 = -15456 (signed)
    // BUF: 50080
    // BUF: -15456

    outputBuffer[6] = bitfieldInsert(c, 0x03U, 3, 2);   // 0xB8 = 184  (unsigned)
    outputBuffer[7] = bitfieldInsert(d, 0x03,  3, 2);    // 0xB8 = -72 (signed)
    // BUF: 184
    // BUF: -72
}
