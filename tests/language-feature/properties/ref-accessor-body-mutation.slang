//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct Container
{
    int first;
    int second;
    int valueStorage;

    property int value
    {
        ref
        {
            first = 3;
            this.second = 4;
            return valueStorage;
        }
    }
}

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Container container;
    container.value = 5;
    outputBuffer[0] = container.first;
    outputBuffer[1] = container.second;
    outputBuffer[2] = container.valueStorage;
}

// CHECK: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
