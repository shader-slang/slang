//TEST:SIMPLE(filecheck=CHECK): -target spirv -experimental-feature

// This test is to check that if user has defined a neural.slang module as well, we will always
// choose the user-defined module.

// CHECK: OpEntryPoint

import neural;

[shader("compute")]
[numthreads(1, 1, 1)]
int computeMain()
{
    return SomeLocalFunc(1);
}
