//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -source-language hlsl -output-using-type -xslang -Wno-30081
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -source-language hlsl -output-using-type -xslang -Wno-30081

// An HLSL-dialect declaration used as a `do`-`while` condition is scoped to the
// condition itself (the body runs before the condition, so it never sees the variable),
// and a `continue` still jumps to the condition, so the condition initializer runs
// before the next body pass.

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// The condition variable is scoped to the condition: the body reads the outer `c`,
// and the outer `c` is restored after the loop.
int doWhileConditionScope()
{
    int c = 50;
    int i = 0;
    int sum = 0;
    do
    {
        sum += c;
        i++;
    } while (bool c = i < 3);
    return sum * 10 + c;
}

// A `continue` in a `do`-`while` jumps to the condition, so the condition initializer
// (`++n < 5`) still runs before the next body pass. If `continue` instead jumped to
// the body, `n % 2 == 0` would hold forever and this loop would not terminate.
int doWhileContinue()
{
    int sum = 0;
    int n = 0;
    do
    {
        if (n % 2 == 0)
            continue;
        sum += n;
    } while (bool c = (++n < 5));
    return sum * 10 + n;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = doWhileConditionScope();
    outputBuffer[1] = doWhileContinue();
}

// CHECK: 1550
// CHECK: 45
