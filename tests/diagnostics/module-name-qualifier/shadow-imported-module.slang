// Importing a module whose declared name is the same as a core-module function
// (`module max;` in max.slang) does not hide that function from the importer
// (shader-slang/slang#14063).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

import max;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 3.0
    outputBuffer[0] = max(3.0f, 1.0f);
    // CHECK: 4.0
    outputBuffer[1] = twice(2.0f);
}
