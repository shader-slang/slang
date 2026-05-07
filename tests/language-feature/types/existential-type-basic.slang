// existential-type-basic.slang

// Test using an interface as an existential type: storing different concrete
// types in the same variable and dispatching dynamically.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

interface IShape
{
    int area();
}

struct Square : IShape
{
    int side;
    int area() { return side * side; }
}

struct Rect : IShape
{
    int w;
    int h;
    int area() { return w * h; }
}

int computeArea(IShape shape)
{
    return shape.area();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Square sq;
    sq.side = 4;

    Rect rc;
    rc.w = 3;
    rc.h = 7;

    // CHECK: 16
    outputBuffer[0] = computeArea(sq);
    // CHECK: 21
    outputBuffer[1] = computeArea(rc);
    // CHECK: 37
    outputBuffer[2] = computeArea(sq) + computeArea(rc);
    // CHECK: 9
    Square sq2;
    sq2.side = 3;
    outputBuffer[3] = computeArea(sq2);
}
