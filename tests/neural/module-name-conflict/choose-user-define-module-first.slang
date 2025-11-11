//TEST:SIMPLE(filecheck=CHECK): -target spirv -experimental-feature

// This test is to check that if user has defined a neural.slang module as well, we will always
// choose the user-defined module.

// CHECK: OpEntryPoint

import neural;

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
     outputBuffer[0] = SomeLocalFunc(1);
}
