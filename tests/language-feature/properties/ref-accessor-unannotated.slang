//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// An unannotated `ref` accessor is implicitly mutating like a setter, so its `this` parameter is
// borrowed by reference and the returned address denotes the caller's storage.

struct Container
{
    int valueStorage;
    property int value
    {
        get { return valueStorage; }
        ref { return valueStorage; }
    }
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Container container;
    container.valueStorage = 4;
    container.value++;
    outputBuffer[0] = container.valueStorage;
}

// CHECK: 5
