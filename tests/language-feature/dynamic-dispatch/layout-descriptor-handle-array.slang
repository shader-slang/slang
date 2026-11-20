// An array of DescriptorHandle fields inside an interface implementation
// tests that the AnyValue marshalling code correctly iterates array elements,
// packing each handle independently at consecutive offsets.

//TEST:COMPARE_COMPUTE(targets=metal+cuda+cpu, filecheck-buffer=CHECK):-output-using-type
//TEST:COMPARE_COMPUTE(targets=vk, filecheck-buffer=CHECK):-output-using-type -render-feature bindless
//TEST:COMPARE_COMPUTE(targets=dx12, filecheck-buffer=CHECK):-output-using-type -profile sm_6_6 -render-feature bindless

interface IIndexedSource
{
    float getFromFirst(int index);
    float getFromSecond(int index);
}

struct ArrayHandleSource : IIndexedSource
{
    StructuredBuffer<float>.Handle handles[2];

    float getFromFirst(int index) { return handles[0][index]; }
    float getFromSecond(int index) { return handles[1][index]; }
}

struct FlatSource : IIndexedSource
{
    float a;
    float b;

    float getFromFirst(int index) { return a; }
    float getFromSecond(int index) { return b; }
}

IIndexedSource makeIndexedSource(
    int kind,
    StructuredBuffer<float>.Handle first,
    StructuredBuffer<float>.Handle second)
{
    if (kind == 0)
    {
        ArrayHandleSource ahs;
        ahs.handles[0] = first;
        ahs.handles[1] = second;
        return ahs;
    }
    FlatSource fs;
    fs.a = 5.0;
    fs.b = 7.0;
    return fs;
}

//TEST_INPUT:ubuffer(data=[100.0 200.0], stride=4):name=firstBuf
uniform StructuredBuffer<float>.Handle firstBuf;

//TEST_INPUT:ubuffer(data=[300.0 400.0], stride=4):name=secondBuf
uniform StructuredBuffer<float>.Handle secondBuf;

//TEST_INPUT:ubuffer(data=[0 1], stride=4):name=kindBuffer
StructuredBuffer<int> kindBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IIndexedSource arraySrc = makeIndexedSource(kindBuffer[0], firstBuf, secondBuf);

    // CHECK: 100.0
    outputBuffer[0] = arraySrc.getFromFirst(0);
    // CHECK: 200.0
    outputBuffer[1] = arraySrc.getFromFirst(1);
    // CHECK: 300.0
    outputBuffer[2] = arraySrc.getFromSecond(0);
    // CHECK: 400.0
    outputBuffer[3] = arraySrc.getFromSecond(1);

    IIndexedSource flatSrc = makeIndexedSource(kindBuffer[1], firstBuf, secondBuf);

    // CHECK: 5.0
    outputBuffer[4] = flatSrc.getFromFirst(0);
    // CHECK: 7.0
    outputBuffer[5] = flatSrc.getFromSecond(0);
}
