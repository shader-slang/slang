//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu

// Default GCC/Clang style packs from LSB to MSB
// struct S { uint a:4; uint b:8; uint c:4; uint d:16; }
// Memory layout (32-bit):
// bits 0-3:   a (0x5)
// bits 4-11:  b (0xAB) 
// bits 12-15: c (0xC)
// bits 16-31: d (0xDEF0)
// Expected: 0xDEF0CAB5

// CHECK: DEF0CAB5

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct S {
    uint a : 4;   // bits 0-3
    uint b : 8;   // bits 4-11
    uint c : 4;   // bits 12-15
    uint d : 16;  // bits 16-31
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

