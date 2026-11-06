//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -source-language hlsl -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -source-language hlsl -output-using-type

// A variable declared in an HLSL `for` loop's initializer must stay invisible
// to code that comes before the loop in the same block.

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int readBeforeForInit(int i)
{
    int value = 0;
    {
        value = i;
        for (int i = 0; i < 2; i++)
        {
        }
    }
    return value;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 7
    outputBuffer[0] = readBeforeForInit(7);
}
