// struct-basic-definition.slang

// Test basic struct definition with fields, construction, and field access.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Point
{
    int x;
    int y;
}

struct Color
{
    int r;
    int g;
    int b;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Construct a struct and access its fields
    Point p;
    p.x = 10;
    p.y = 20;

    // CHECK: 10
    outputBuffer[0] = p.x;

    // CHECK: 20
    outputBuffer[1] = p.y;

    // Construct another struct and verify field access
    Color c;
    c.r = 255;
    c.g = 128;
    c.b = 64;

    // CHECK: 255
    outputBuffer[2] = c.r;

    // CHECK: 192
    outputBuffer[3] = c.g + c.b;
}
