//TEST(compute):COMPARE_COMPUTE:-cpu -compile-arg -msvc-style-bitfield-packing
//TEST(compute):COMPARE_COMPUTE:-cpu -compile-arg -bitfield-packing-rules -compile-arg msvc

// The deprecated option creates separate backing fields for different type sizes,
// like MSVC, but packs the fields within each backing integer MSB-first.
// struct MixedSizes { uint8_t a:4; uint8_t b:4; uint16_t c:8; uint16_t d:8; }
// With the legacy flag, the uint8_t backing is 0xAB and the uint16_t backing is 0xCDEF.
// With -bitfield-packing-rules msvc, their LSB-first values are 0xBA and 0xEFCD.
// The two backing fields are separate under both modes because the type size changes.

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct MixedSizes {
    uint8_t a : 4;   // First backing field (uint8_t)
    uint8_t b : 4;   // Same backing field
    uint16_t c : 8;  // New backing field (uint16_t) due to type-size change
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

