//TEST:SIMPLE(filecheck=COMPILE): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=COMPILE): -target spirv -stage compute -entry computeMain
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -source-language hlsl -output-using-type
// COMPILE: result code = 0

// HLSL classes are values: copies and by-value parameters have independent storage.
// These declarations must also lower to shader value types without requiring `new`.
// https://github.com/shader-slang/slang/issues/13495

class Value
{
    int x;
};

Value changeCopy(Value value)
{
    value.x = 3;
    return value;
}

class Outer
{
    class Inner
    {
        int x;
    };
    Inner inner;
};

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Value original = { 1 };
    Value copy = original;
    copy.x = 2;
    Value returned = changeCopy(original);

    // CHECK: 1
    outputBuffer[0] = original.x;
    // CHECK: 2
    outputBuffer[1] = copy.x;
    // CHECK: 3
    outputBuffer[2] = returned.x;

    // A local class and a nested class use the same declaration nesting rules as structs.
    class Local
    {
        int x;
    };
    Local local = { 4 };
    Outer outer = { { local.x } };
    // CHECK: 4
    outputBuffer[3] = outer.inner.x;
}
