// struct-default-member-values.slang

// Test default values for struct members, used with default construction.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Config
{
    int width = 640;
    int height = 480;
    int depth = 1;
}

static int kOffset = 7;

struct Initialized
{
    int a = 5;
    int b = 10;
    int c = kOffset + 8;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Default-constructed struct uses default member values
    Config cfg = Config();

    // CHECK: 640
    outputBuffer[0] = cfg.width;

    // CHECK: 480
    outputBuffer[1] = cfg.height;

    // CHECK: 1
    outputBuffer[2] = cfg.depth;

    // Struct with expression default values
    Initialized init = Initialized();

    // CHECK: 15
    outputBuffer[3] = init.c;
}
