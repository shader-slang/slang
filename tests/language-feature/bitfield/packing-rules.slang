//TEST(compute):COMPARE_COMPUTE:-cpu
//TEST(compute):COMPARE_COMPUTE:-cpu -compile-arg -bitfield-packing-rules -compile-arg default
//TEST(compute):COMPARE_COMPUTE:-cpu -compile-arg -bitfield-packing-rules -compile-arg msvc
//TEST(compute):COMPARE_COMPUTE:-cpu -compile-arg -bitfield-packing-rules -compile-arg msb-first-msvc

// The first word distinguishes LSB-first from MSB-first packing within one backing uint.
// In Mixed, default rules fit both fields in one uint16_t backing field.
// Both modes with the type-size split allocate separate uint8_t and uint16_t backing fields.
//
//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct Same
{
    uint a : 4;
    uint b : 8;
    uint c : 4;
    uint d : 16;
};

struct Mixed
{
    uint8_t a : 4;
    uint16_t b : 4;
};

[numthreads(1, 1, 1)]
void computeMain()
{
    Same same;
    same.a = 0x5;
    same.b = 0xAB;
    same.c = 0xC;
    same.d = 0xDEF0;

    outputBuffer[0] = *((uint*)__getAddress(same));
    outputBuffer[1] = sizeof(Mixed);
}
