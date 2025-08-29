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

groupshared MixedSizes m;

typealias GroupSharedPtr<T> = Ptr<T, Access::ReadWrite, AddressSpace::GroupShared>;

[numthreads(1, 1, 1)]
void computeMain()
{
    m.a = 0xA;
    m.b = 0xB;
    m.c = 0xCD;
    m.d = 0xEF;

    // Read the two backing fields separately
    GroupSharedPtr<uint8_t> p8 = (GroupSharedPtr<uint8_t>)__getAddress(m);
    GroupSharedPtr<uint16_t> p16 = (GroupSharedPtr<uint16_t>)( ((GroupSharedPtr<uint8_t>)__getAddress(m)) + 2); // Skip uint8_t + padding

    outputBuffer[0] = (uint)*p8;
    outputBuffer[1] = (uint)*p16;
}

