// interface-multiple-conformance.slang

// Test struct conforming to multiple interfaces.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

interface IPosition
{
    int getX();
}

interface ISize
{
    int getArea();
}

struct Rect : IPosition, ISize
{
    int x;
    int width;
    int height;

    int getX() { return x; }
    int getArea() { return width * height; }
}

int readPosition<T : IPosition>(T obj)
{
    return obj.getX();
}

int readArea<T : ISize>(T obj)
{
    return obj.getArea();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    Rect r;
    r.x = 10;
    r.width = 3;
    r.height = 4;

    outputBuffer[0] = readPosition(r);
    // CHECK: 10

    outputBuffer[1] = readArea(r);
    // CHECK: 12
}
