// struct-constructors.slang

// Test explicit __init constructors (default and parameterized).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Vec2
{
    int x;
    int y;

    // Default constructor
    __init()
    {
        x = 0;
        y = 0;
    }

    // Parameterized constructor
    __init(int ax, int ay)
    {
        x = ax;
        y = ay;
    }
}

struct Counter
{
    int value;
    int step;

    __init(int startValue)
    {
        value = startValue;
        step = 1;
    }

    __init(int startValue, int stepSize)
    {
        value = startValue;
        step = stepSize;
    }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Use default constructor
    Vec2 v0 = Vec2();

    // CHECK: 0
    outputBuffer[0] = v0.x + v0.y;

    // Use parameterized constructor
    Vec2 v1 = Vec2(3, 7);

    // CHECK: 10
    outputBuffer[1] = v1.x + v1.y;

    // Single-parameter constructor
    Counter c1 = Counter(100);

    // CHECK: 101
    outputBuffer[2] = c1.value + c1.step;

    // Two-parameter constructor
    Counter c2 = Counter(50, 5);

    // CHECK: 55
    outputBuffer[3] = c2.value + c2.step;
}
