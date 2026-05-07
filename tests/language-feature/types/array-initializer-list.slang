// array-initializer-list.slang

// Test initializer lists for arrays of scalars, vectors, and structs.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Pair
{
    int x;
    int y;
};

[numthreads(1, 1, 1)]
void computeMain()
{
    // Array of scalars
    int scalars[] = {5, 10, 15};

    // CHECK: 30
    outputBuffer[0] = scalars[0] + scalars[1] + scalars[2];

    // Array of vectors
    int2 vectors[] = { int2(1, 2), int2(3, 4) };

    // CHECK: 4
    outputBuffer[1] = vectors[0].x + vectors[1].x;

    // CHECK: 6
    outputBuffer[2] = vectors[0].y + vectors[1].y;

    // Array of structs
    Pair pairs[] = { {10, 20}, {30, 40} };

    // CHECK: 40
    outputBuffer[3] = pairs[0].x + pairs[1].x;

    // CHECK: 60
    outputBuffer[4] = pairs[0].y + pairs[1].y;

    // Float array (result cast to int)
    float floats[] = {1.5, 2.5, 3.0};
    int sum = int(floats[0] + floats[1] + floats[2]);

    // CHECK: 7
    outputBuffer[5] = sum;
}
