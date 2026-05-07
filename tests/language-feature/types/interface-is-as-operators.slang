// interface-is-as-operators.slang

// Test `is` operator for type testing and `as` for downcasting.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

interface IShape
{
    int getArea();
}

struct Square : IShape
{
    int side;
    int getArea() { return side * side; }
}

struct Triangle : IShape
{
    int base;
    int height;
    int getArea() { return (base * height) / 2; }
}

int tryGetSquareSide<T : IShape>(T shape)
{
    if (shape is Square)
    {
        let sq = shape as Square;
        if (sq.hasValue)
            return sq.value.side;
    }
    return -1;
}

int identify<T : IShape>(T shape)
{
    if (T is Square)
        return 1;
    if (T is Triangle)
        return 2;
    return 0;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    Square sq;
    sq.side = 4;

    Triangle tri;
    tri.base = 6;
    tri.height = 3;

    // is operator: identify type
    outputBuffer[0] = identify(sq);
    // CHECK: 1

    outputBuffer[1] = identify(tri);
    // CHECK: 2

    // as operator: downcast and extract
    outputBuffer[2] = tryGetSquareSide(sq);
    // CHECK: 4

    outputBuffer[3] = tryGetSquareSide(tri);
    // CHECK: -1
}
