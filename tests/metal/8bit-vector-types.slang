//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

RWStructuredBuffer<uint> outputBuffer;

uint8_t2 createUChar2()
{
    return uint8_t2(1, 2);
}

int8_t2 createChar2()
{
    return int8_t2(3, 4);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: uchar2
    // CHECK: char2
    uint8_t2 u8v2 = createUChar2();
    int8_t2 i8v2 = createChar2();
    
    outputBuffer[0] = u8v2.x + i8v2.x;
}