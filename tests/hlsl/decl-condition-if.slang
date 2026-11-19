//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -source-language hlsl -output-using-type -xslang -Wno-30081
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -source-language hlsl -output-using-type -xslang -Wno-30081

// A declaration used as an `if` condition has the condition value of the declared
// variable, converted to bool, and the variable is visible in both arms of the
// statement but not after it.

// The tests use non-`bool` condition types, whose conversion to `bool` is
// exactly the feature under test, so the 30081 "implicit conversion not recommended"
// warning (which fires for any non-`bool` condition value) is disabled rather than
// heeded.

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// The condition value is the declared variable, converted to bool.
int ifConditionValue()
{
    int c = 3;
    if (int count = c)
        return count;
    return 0;
}

// The declared variable is visible in both the `then` and `else` arms.
int ifElseVisibility()
{
    if (int count = 0)
        return 1;
    else
        return count + 10;
}

// The declared variable is visible in an `else if` condition and arm.
int elseIfChain()
{
    int x = 0;
    if (int a = x)
        return a;
    else if (int b = 5)
        return b + a;
    return 0;
}

// The declared variable is scoped to the statement: an outer variable with the same
// name is shadowed only inside the statement and restored after it.
int ifScoping()
{
    int count = 40;
    int before = count;
    if (int count = 1)
        before += count;
    return before * 10 + count;
}

// A float condition converts through the same predicate path as `if (f)`.
int ifFloatCondition()
{
    if (float f = 0.5)
        return 1;
    if (float f = 0.0)
        return 2;
    return 3;
}

// The declaration's initializer runs exactly once per execution of the statement, even
// when the condition is false: `count` is 1 after the statement, not 2 or 0.
int ifInitializerRunsOnce()
{
    int count = 0;
    if (bool taken = ++count > 5)
        return 100;
    return count;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = ifConditionValue();
    outputBuffer[1] = ifElseVisibility();
    outputBuffer[2] = elseIfChain();
    outputBuffer[3] = ifScoping();
    outputBuffer[4] = ifFloatCondition();
    outputBuffer[5] = ifInitializerRunsOnce();
}

// CHECK: 3
// CHECK: 10
// CHECK: 5
// CHECK: 450
// CHECK: 1
// CHECK: 1
