//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -source-language hlsl -output-using-type -xslang -Wno-30081
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -source-language hlsl -output-using-type -xslang -Wno-30081

// An HLSL-dialect declaration used as a `while` condition follows C++ rules: the
// declaration runs before each test, and the variable is visible in the body.

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// Each iteration re-initializes the condition variable before testing it.
int whileReinitializes()
{
    int n = 3;
    int sum = 0;
    while (int i = n--)
        sum += i;
    return sum * 10 + n;
}

// The condition variable is visible in the body; an outer variable with the same
// name is shadowed only inside the statement and restored after it.
int whileScoping()
{
    int i = 60;
    int n = 2;
    int sum = 0;
    while (int i = n--)
        sum += 1;
    return sum * 10 + i;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = whileReinitializes();
    outputBuffer[1] = whileScoping();
}

// CHECK: 59
// CHECK: 80
