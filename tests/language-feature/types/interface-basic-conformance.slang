// interface-basic-conformance.slang

// Test basic interface definition and struct conformance.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type

interface IValue
{
    int getValue();
}

struct MyStruct : IValue
{
    int data;
    int getValue() { return data; }
}

int callGetValue<T : IValue>(T obj)
{
    return obj.getValue();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    MyStruct s;
    s.data = 42;
    outputBuffer[0] = callGetValue(s);
    // CHECK: 42

    MyStruct s2;
    s2.data = 7;
    outputBuffer[1] = s2.getValue();
    // CHECK: 7
}
