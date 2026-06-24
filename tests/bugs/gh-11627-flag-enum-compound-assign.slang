//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk    -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-slang -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cuda  -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgsl  -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-hlsl  -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu   -entry computeMain -output-using-type

enum MyFlags
{
    A = 1,
    B = 2,
    C = 4,
};

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
void computeMain()
{
    MyFlags test1 = {};

    // basic and/or/xor
    test1 = MyFlags::A;
    test1 |= MyFlags::B;
    outputBuffer[0] = (uint)test1;  // A|B
// BUF: 3

    test1 = MyFlags::A | MyFlags::C;
    test1 &= MyFlags::A | MyFlags::B;
    outputBuffer[1] = (uint)test1;  // A
// BUF: 1

    test1 = MyFlags::A | MyFlags::C;
    test1 ^= MyFlags::A | MyFlags::B;
    outputBuffer[2] = (uint)test1;  // B|C
// BUF: 6

    test1 = MyFlags::A;

    test1 |= test1 ^= MyFlags::B;
    outputBuffer[3] = (uint)test1;  // A|B
// BUF: 3

}
