// A function with the same name as an imported module resolves to the
// function, whether it is declared in another imported module or in this
// module (shader-slang/slang#14063).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

import foo;
import mod_f;

int foo(int x) { return x * 10; }

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 2.0
    outputBuffer[0] = foo(1.0f);
    // CHECK: 30.0
    outputBuffer[1] = float(foo(3));
}
