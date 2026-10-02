// Shared C++/Slang source. The reference runner defines CPU_REFERENCE for GCC.
int sequential(int x)
{
    int y = x; y += 2; y *= 3; // CASE: sequential
    return y;
}

int sameLine(int x) { if (x > 0) return 10; else return 20; } // CASE: same_line

int positiveArm(int x) { return x + 3; }
int negativeArm(int x) { return x - 3; }

int choose(int x)
{
    return x > 0
        ? positiveArm(x) // CASE: true_arm
        : negativeArm(x); // CASE: false_arm
}

int logical(int x)
{
    if (x < 0)
    {
        if (x > 100 && x < 200) // CASE: skipped_rhs
            return 1;
    }
    return 0;
}

int mixed(int x)
{
    if (!(x > 0 && (x < 2 || x > 100))) // CASE: mixed
        return 7;
    return 9;
}

int loop(int x)
{
    int result = 0;
    for (int i = 0; i < x; ++i) { result += i; } // CASE: loop_line
    return result;
}

int cases(int x)
{
    int result = 0;
    switch (x) // CASE: switch
    {
    case 0:
        result += 1;
    case 1:
        result += 2;
        break;
    default:
        result += 4;
        break;
    }
    return result;
}

#ifndef CPU_REFERENCE
StructuredBuffer<int> inputBuffer;
RWStructuredBuffer<int> outputBuffer;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int x = inputBuffer[tid.x];
    outputBuffer[tid.x] = sequential(x) + sameLine(x) + choose(x) + logical(x)
        + mixed(x) + loop(x) + cases(x);
}
#endif
