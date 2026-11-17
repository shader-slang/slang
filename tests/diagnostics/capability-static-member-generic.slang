// A [require] on a static member function must be enforced when the call goes
// through a generic type, matching the non-generic spelling of the same call.
// Regression test for https://github.com/shader-slang/slang/issues/14430.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage compute -entry computeMain -DTEST_GENERIC
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage compute -entry computeMain -DTEST_NON_GENERIC

RWStructuredBuffer<float> outputBuffer;

struct S<T>
{
    [require(spirv)]
    static uint SF() { __target_switch { case spirv: return 1; } }
}

struct N
{
    [require(spirv)]
    static uint SF() { __target_switch { case spirv: return 1; } }
}

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain()
{
#ifdef TEST_GENERIC
    // CHECK: error 36107
    outputBuffer[0] = S<float>.SF();
#endif
#ifdef TEST_NON_GENERIC
    // CHECK: error 36107
    outputBuffer[0] = N.SF();
#endif
}
