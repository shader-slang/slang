//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu   -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk    -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-slang -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cuda  -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgsl  -entry computeMain -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-hlsl  -entry computeMain -output-using-type

[Flags]
enum MyFlags
{
    A,
    B,
    C,
};

enum MyEnum
{
    A = 1,
    B = 2,
    C = 4,
};

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
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

    MyEnum test2 = {};

    // basic and/or/xor
    test2 = MyEnum::A;
    test2 |= MyEnum::B;
    outputBuffer[4] = (uint)test2;  // A|B
// BUF: 3

    test2 = MyEnum::A | MyEnum::C;
    test2 &= MyEnum::A | MyEnum::B;
    outputBuffer[5] = (uint)test2;  // A
// BUF: 1

    test2 = MyEnum::A | MyEnum::C;
    test2 ^= MyEnum::A | MyEnum::B;
    outputBuffer[6] = (uint)test2;  // B|C
// BUF: 6

    test2 = MyEnum::A;
    test2 |= test2 ^= MyEnum::B;
    outputBuffer[7] = (uint)test2;  // A|B
// BUF: 3

}
