//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compile-arg -msvc-style-bitfield-packing

// MSVC style packs from MSB to LSB
// struct S { uint a:4; uint b:8; uint c:4; uint d:16; }
// Memory layout (32-bit):
// bits 28-31: a (0x5)
// bits 20-27: b (0xAB) 
// bits 16-19: c (0xC)
// bits 0-15:  d (0xDEF0)
// Expected: 0x5ABCDEF0

// CHECK: 5ABCDEF0

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct S {
    uint a : 4;   // bits 28-31 (MSB side)
    uint b : 8;   // bits 20-27
    uint c : 4;   // bits 16-19
    uint d : 16;  // bits 0-15
};

[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    s.a = 0x5;
    s.b = 0xAB;
    s.c = 0xC;
    s.d = 0xDEF0;

    // Write the struct to memory and read it back as uint
    outputBuffer[0] = *((uint*)&s);
}

