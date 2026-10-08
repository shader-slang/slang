//TEST:SIMPLE(filecheck=COMPILE): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=COMPILE): -target spirv -stage compute -entry computeMain
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -source-language hlsl -output-using-type
// COMPILE: result code = 0

// Interface conformance is a Slang extension accepted in HLSL mode for value types.
// An HLSL class must satisfy a generic constraint through the same checking as a struct.
interface IGetValue
{
    int getValue() const;
}

class Element : IGetValue
{
    int value;

    int getValue() const
    {
        return value;
    }
};

int useInterface<T : IGetValue>(T element)
{
    return element.getValue();
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Element element = { 7 };
    // CHECK: 7
    outputBuffer[0] = useInterface(element);
}
