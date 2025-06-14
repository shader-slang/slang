//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

import this_file_does_not_exist;

import empty;


//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=16):out
RWStructuredBuffer<int> outputBuffer;

// CHECK-NOT: error 1: {{.*}} 'empty.slang'

[numthreads(4, 4, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = dispatchThreadID.x;
    outputBuffer[index] = index;
}

