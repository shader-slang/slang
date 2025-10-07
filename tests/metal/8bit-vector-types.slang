//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4, counter=1):out,name=outputBuffer
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
    // MTL: uchar2
    // MTL: char2
    uint8_t2 u8v2 = createUChar2();
    int8_t2 i8v2 = createChar2();
    
    // BUF: {{^}}4{{$}}
    outputBuffer[0] = u8v2.x + i8v2.x;
}
