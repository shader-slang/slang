//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -compile-arg -msvc-style-bitfield-packing

// MSVC packing with mixed type sizes
// CHECK:      15
// CHECK-NEXT: 255
// CHECK-NEXT: 4095
// CHECK-NEXT: 8

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct MixedTypes {
    uint8_t a : 4;   // Uses uint8_t backing, bits 7-4
    uint8_t b : 4;   // Same backing, bits 3-0
    uint16_t c : 8;  // New uint16_t backing due to type change, bits 15-8
    uint16_t d : 8;  // Same backing, bits 7-0
    uint32_t e : 12; // New uint32_t backing due to type change, bits 31-20
    uint32_t f : 12; // Same backing, bits 19-8
    uint32_t g : 8;  // Same backing, bits 7-0
};

[numthreads(1, 1, 1)]
void computeMain()
{
    MixedTypes m;
    m.a = 15;   // 0xF
    m.b = 15;   // 0xF
    m.c = 255;  // 0xFF
    m.d = 255;  // 0xFF
    m.e = 4095; // 0xFFF
    m.f = 4095; // 0xFFF
    m.g = 255;  // 0xFF

    outputBuffer[0] = m.a;
    outputBuffer[1] = m.c;
    outputBuffer[2] = m.e;
    // Verify struct size includes all three backing fields
    outputBuffer[3] = sizeof(MixedTypes);
}

