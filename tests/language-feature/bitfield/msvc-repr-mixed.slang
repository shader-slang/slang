//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compile-arg -msvc-style-bitfield-packing

// MSVC creates separate backing fields for different type sizes
// struct MixedSizes { uint8_t a:4; uint8_t b:4; uint16_t c:8; uint16_t d:8; }
// First backing (uint8_t):  a=0xA (bits 4-7), b=0xB (bits 0-3) => 0xAB
// Second backing (uint16_t): c=0xCD (bits 8-15), d=0xEF (bits 0-7) => 0xCDEF
// Memory layout: [0xAB, padding, 0xCDEF] or [0xAB, padding, 0xEF, 0xCD] depending on endianness

// CHECK: AB
// CHECK-NEXT: CDEF

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct MixedSizes {
    uint8_t a : 4;   // First backing field (uint8_t)
    uint8_t b : 4;   // Same backing field
    uint16_t c : 8;  // New backing field (uint16_t) due to type change
    uint16_t d : 8;  // Same backing field
};

[numthreads(1, 1, 1)]
void computeMain()
{
    MixedSizes m;
    m.a = 0xA;
    m.b = 0xB;
    m.c = 0xCD;
    m.d = 0xEF;

    // Read the two backing fields separately
    uint8_t* p8 = (uint8_t*)&m;
    uint16_t* p16 = (uint16_t*)((uint8_t*)&m + 2); // Skip uint8_t + padding

    outputBuffer[0] = uint(*p8);
    outputBuffer[1] = uint(*p16);
}

