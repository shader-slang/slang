//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -source-language hlsl -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -source-language hlsl -output-using-type

// An HLSL-dialect declaration used as an `if` condition follows C++ rules: the condition
// value is the declared variable converted to bool, and the variable is visible in both
// arms of the statement but not after it.

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// The condition value is the declared variable, converted to bool.
int ifConditionValue()
{
    int c = 3;
    if (int count = c)
        return count;
    return 0;
}

// The condition variable is visible in both the `then` and `else` arms.
int ifElseVisibility()
{
    if (int count = 0)
        return 1;
    else
        return count + 10;
}

// The condition variable is visible in an `else if` condition and arm.
int elseIfChain()
{
    int x = 0;
    if (int a = x)
        return a;
    else if (int b = 5)
        return b + a;
    return 0;
}

// The condition variable is scoped to the statement: an outer variable with the same
// name is shadowed only inside the statement and restored after it.
int ifScoping()
{
    int count = 40;
    int before = count;
    if (int count = 1)
        before += count;
    return before * 10 + count;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = ifConditionValue();
    outputBuffer[1] = ifElseVisibility();
    outputBuffer[2] = elseIfChain();
    outputBuffer[3] = ifScoping();
}

// CHECK: 3
// CHECK: 10
// CHECK: 5
// CHECK: 450
