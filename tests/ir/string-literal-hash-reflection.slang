//TEST:REFLECTION:-stage compute -entry computeMain -target hlsl -no-codegen

import string_literal_module;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(
    uint tid   : SV_DispatchThreadIndex)
{
    int value = doSomethingElse() + getStringHash("Hello \t\n\0x083 World");   
    outputBuffer[tid] = value;
}

