//TEST:SIMPLE(filecheck=COMPILE): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=COMPILE): -target spirv -stage compute -entry computeMain
// COMPILE: result code = 0

// HLSL classes share the struct parser, including Slang's extensions to HLSL syntax.
// Inline generic parameters and anonymous declarations must use the shared struct grammar.
// https://github.com/shader-slang/slang/issues/13495
class Box<T>
{
    T value;
};

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Box<int> box = { 5 };
    class
    {
        int value;
    } anonymous = { box.value };
    outputBuffer[0] = anonymous.value;
}
