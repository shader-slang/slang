// type-conversions-implicit.slang

// Test implicit conversions between scalar types.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // int to float (implicit)
    int i = 7;
    float fromInt = i;

    // CHECK: 7.0
    outputBuffer[0] = fromInt;

    // uint to float (implicit)
    uint u = 42u;
    float fromUint = u;

    // CHECK: 42.0
    outputBuffer[1] = fromUint;

    // int16_t to float (implicit widening)
    int16_t s16 = int16_t(100);
    float fromInt16 = s16;

    // CHECK: 100.0
    outputBuffer[2] = fromInt16;

    // int8_t to int to float
    int8_t i8 = int8_t(25);
    int fromI8 = i8;
    float result8 = fromI8;

    // CHECK: 25.0
    outputBuffer[3] = result8;

    // int to double (implicit widening)
    int bigI = 999;
    double fromIntD = bigI;
    float resultD = float(fromIntD);

    // CHECK: 999.0
    outputBuffer[4] = resultD;

    // half to float (implicit widening)
    half h = half(3.5);
    float fromHalf = h;

    // CHECK: 3.5
    outputBuffer[5] = fromHalf;
}
