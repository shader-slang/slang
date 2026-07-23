// scalar-integer-types.slang

// Test that all integer scalar types can be declared, assigned, and used in arithmetic.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // int8_t
    int8_t a8 = int8_t(10);
    int8_t b8 = int8_t(3);
    int result8 = int(a8 + b8);

    // CHECK: 13
    outputBuffer[0] = result8;

    // int16_t
    int16_t a16 = int16_t(200);
    int16_t b16 = int16_t(56);
    int result16 = int(a16 + b16);

    // CHECK: 256
    outputBuffer[1] = result16;

    // int (32-bit)
    int a32 = 1000;
    int b32 = 234;

    // CHECK: 1234
    outputBuffer[2] = a32 + b32;

    // int64_t
    int64_t a64 = int64_t(100000);
    int64_t b64 = int64_t(23456);
    int result64 = int(a64 + b64);

    // CHECK: 123456
    outputBuffer[3] = result64;

    // uint8_t
    uint8_t ua8 = uint8_t(250);
    uint8_t ub8 = uint8_t(5);
    int uresult8 = int(ua8 - ub8);

    // CHECK: 245
    outputBuffer[4] = uresult8;

    // uint16_t
    uint16_t ua16 = uint16_t(60000);
    uint16_t ub16 = uint16_t(5000);
    int uresult16 = int(ua16 - ub16);

    // CHECK: 55000
    outputBuffer[5] = uresult16;

    // uint (32-bit)
    uint ua32 = 3000000000u;
    uint ub32 = 1000000000u;
    int uresult32 = int(ua32 - ub32);

    // CHECK: 2000000000
    outputBuffer[6] = uresult32;

    // uint64_t
    uint64_t ua64 = uint64_t(9000000000);
    uint64_t ub64 = uint64_t(8000000000);
    int uresult64 = int(ua64 - ub64);

    // CHECK: 1000000000
    outputBuffer[7] = uresult64;
}
