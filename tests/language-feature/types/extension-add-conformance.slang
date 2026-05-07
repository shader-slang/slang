// extension-add-conformance.slang

// Test that an `extension` can add interface conformance to an existing type,
// making it usable in generic contexts that require the interface.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

interface IStringLength
{
    int getLength();
}

struct MyData
{
    int a;
    int b;
    int c;
}

extension MyData : IStringLength
{
    int getLength()
    {
        return 3;
    }
}

int queryLength<T : IStringLength>(T val)
{
    return val.getLength();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    MyData d = { 10, 20, 30 };

    // CHECK: 3
    outputBuffer[0] = d.getLength();

    // CHECK: 3
    outputBuffer[1] = queryLength(d);
}
