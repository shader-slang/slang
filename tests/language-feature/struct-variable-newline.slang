//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF): -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Test case for issue 8187: Variable name on new line after struct declaration
static const struct
{
    int value;
}
structOnNewLine = { 1 };

// Test another variable on new line
static const struct
{
    int data;
}
secondStruct = { 2 };

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    // BUF: 1
    outputBuffer[0] = structOnNewLine.value;
    // BUF: 2
    outputBuffer[1] = secondStruct.data;
    // BUF: 3
    outputBuffer[2] = 3;
    // BUF: 4
    outputBuffer[3] = 4;
}