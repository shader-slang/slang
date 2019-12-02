//TEST(compute):SIMPLE: -stage compute -entry main -target hlsl

import string_literal_module;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void main(
    uint tid   : SV_DispatchThreadIndex)
{
	//doSomething("Hello \t\n\0x083 World");
    
    int value = doSomethingElse() + getStringHash("Hello \t\n\0x083 World");
    
    outputBuffer[tid] = value;
}

