//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type
module user;

import lib;

//TEST_INPUT: set myVar = 444
[MyAttr(2)]
uniform int myVar;

//TEST_INPUT: set output = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void computeMain()
{
    // CHECK: 444
    output[0] = myVar;
}