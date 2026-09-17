//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct Container
{
    int values[2];

    __subscript(int index) -> int
    {
        ref { return values[index]; }
    }
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Container container;
    container.values[1] = 6;
    container[1] += 3;
    outputBuffer[0] = container.values[1];
}

// CHECK: 9
