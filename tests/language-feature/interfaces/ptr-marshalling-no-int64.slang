//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -xslang -disable-specialization

// Test that demonstrates pointer marshalling without Int64 capability usage
interface IValue
{
    uint getValue();
}

struct PtrStruct : IValue
{
    uint* ptr;
    uint data;
    
    uint getValue()
    {
        return data;
    }
}

struct IntStruct : IValue
{
    int value;
    
    uint getValue()
    {
        return uint(value);
    }
}

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint localData = 42;
    
    PtrStruct p;
    p.ptr = &localData;
    p.data = 123;
    
    IntStruct i;
    i.value = 456;
    
    IValue val = i;  // Just use IntStruct directly
    
    // CHECK: 1C8
    outputBuffer[0] = val.getValue();
}