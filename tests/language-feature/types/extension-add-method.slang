// extension-add-method.slang

// Test that an `extension` can add methods to an existing type.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

struct Vec2
{
    int x;
    int y;
}

extension Vec2
{
    int lengthSquared()
    {
        return x * x + y * y;
    }

    int dot(Vec2 other)
    {
        return x * other.x + y * other.y;
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    Vec2 a = { 3, 4 };
    Vec2 b = { 1, 2 };

    // CHECK: 25
    outputBuffer[0] = a.lengthSquared();

    // CHECK: 11
    outputBuffer[1] = a.dot(b);
}
