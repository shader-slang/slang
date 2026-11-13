// Qualified access through a `public namespace` in an imported module works;
// it is the supported alternative to using a module name as a qualifier
// (shader-slang/slang#14063).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

import mod_ns;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 6.0
    outputBuffer[0] = NS.twice(3.0f);
}
