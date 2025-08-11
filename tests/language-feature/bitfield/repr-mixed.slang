//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu

// Default packing keeps all fields in one backing field when they fit
// struct MixedSizes { uint8_t a:4; uint8_t b:4; uint16_t c:8; uint16_t d:8; }
// All packed in single uint32_t backing:
// bits 0-3:   a (0xA)
// bits 4-7:   b (0xB)
// bits 8-15:  c (0xCD)
// bits 16-23: d (0xEF)
// Expected: 0x00EFCDBA (top 8 bits unused)

// CHECK: EFCDBA

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct MixedSizes {
    uint8_t a : 4;   // bits 0-3
    uint8_t b : 4;   // bits 4-7
    uint16_t c : 8;  // bits 8-15
    uint16_t d : 8;  // bits 16-23
};

[numthreads(1, 1, 1)]
void computeMain()
{
    MixedSizes m;
    m.a = 0xA;
    m.b = 0xB;
    m.c = 0xCD;
    m.d = 0xEF;

    // With default packing, all fields are in one backing field
    outputBuffer[0] = *((uint*)&m) & 0xFFFFFF; // Mask to 24 bits
}

