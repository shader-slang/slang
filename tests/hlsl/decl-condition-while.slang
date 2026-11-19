//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -source-language hlsl -output-using-type -xslang -Wno-30081
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -source-language hlsl -output-using-type -xslang -Wno-30081

// A declaration used as a `while` condition has its declaration run before each test,
// and the variable is visible in the body.

// The tests use non-`bool` condition types, whose conversion to `bool` is
// exactly the feature under test, so the 30081 "implicit conversion not recommended"
// warning (which fires for any non-`bool` condition value) is disabled rather than
// heeded.

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
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

// A `continue` in a `while` jumps to the condition, so the condition initializer
// (`++n < 5`) still runs before the next body pass. If `continue` instead skipped the
// condition, `n` would stay 2 forever and this loop would not terminate.
int whileContinue()
{
    int sum = 0;
    int n = 0;
    while (bool c = ++n < 5)
    {
        if (n % 2 == 0)
            continue;
        sum += n;
    }
    return sum * 10 + n;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = whileReinitializes();
    outputBuffer[1] = whileScoping();
    outputBuffer[2] = whileContinue();
}

// CHECK: 59
// CHECK: 80
// CHECK: 45
