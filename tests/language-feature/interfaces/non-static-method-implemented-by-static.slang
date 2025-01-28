//TEST(compute):COMPARE_COMPUTE_EX:-vk -compute  -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4);
RWStructuredBuffer<int> outputBuffer;

interface Base
{
    int getValue();
    int getValue(float a);
}

struct Impl1 : Base
{
    // This is static and allowed to implement interface's non-static method.
    static int getValue() { return 5; }
    int getValue(float a) { return int(a) + 5; }
}

struct Impl2 : Base
{
    // This is static with one default parameter and allowed to implement interface's non-static method.
    static int getValue(int a = 3) { return a + 5; }
    int getValue(float a) { return int(a) + 5; }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Impl1 impl1;
    Impl2 impl2;

    outputBuffer[0] = impl1.getValue();
    outputBuffer[1] = impl2.getValue();
    outputBuffer[2] = impl2.getValue(5);
}
