//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target hlsl -entry computeMain -stage compute

import this_file_does_not_exist;

import empty;


//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=16):out
RWStructuredBuffer<int> outputBuffer;

// The error must be attributed to the missing file, not to the import that follows it.
// CHECK: error[E00001]: cannot open file 'this_file_does_not_exist.slang'
// CHECK-NOT: cannot open file 'empty.slang'

[numthreads(4, 4, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = dispatchThreadID.x;
    outputBuffer[index] = index;
}

