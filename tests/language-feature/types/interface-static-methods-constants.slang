// interface-static-methods-constants.slang

// Test static methods and static constants in interfaces.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

interface IConfig
{
    static const int kDefault;
    static int getScaled(int x);
}

struct ConfigA : IConfig
{
    static const int kDefault = 10;
    static int getScaled(int x) { return x * 2; }
}

struct ConfigB : IConfig
{
    static const int kDefault = 20;
    static int getScaled(int x) { return x * 3; }
}

int readDefault<T : IConfig>()
{
    return T.kDefault;
}

int callScaled<T : IConfig>(int x)
{
    return T.getScaled(x);
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    outputBuffer[0] = readDefault<ConfigA>();
    // CHECK: 10

    outputBuffer[1] = readDefault<ConfigB>();
    // CHECK: 20

    outputBuffer[2] = callScaled<ConfigA>(5);
    // CHECK: 10

    outputBuffer[3] = callScaled<ConfigB>(5);
    // CHECK: 15
}
