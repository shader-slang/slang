//TEST:SIMPLE(filecheck=COMPILE): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=COMPILE): -target spirv -stage compute -entry computeMain
// COMPILE: result code = 0

// HLSL classes must work as buffer elements just like structs.
// https://github.com/shader-slang/slang/issues/13495
class Element
{
    int value;
};

ConstantBuffer<Element> constants;
StructuredBuffer<Element> inputBuffer;
RWStructuredBuffer<Element> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Element element = inputBuffer[tid.x];
    element.value += constants.value;
    outputBuffer[tid.x] = element;
}
