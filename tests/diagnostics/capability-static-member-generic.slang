// A [require] on a static member must be enforced when the member is reached
// through a generic type, matching the non-generic spelling — for the member
// declaration's own requirements (functions) and for requirements carried by
// the member's type.
// Regression test for https://github.com/shader-slang/slang/issues/14430

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage compute -entry computeMain -DTEST_GENERIC_FUNCTION
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage compute -entry computeMain -DTEST_NON_GENERIC_FUNCTION
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage compute -entry computeMain -DTEST_TYPE_NON_GENERIC
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage compute -entry computeMain -DTEST_TYPE_GENERIC

RWStructuredBuffer<float> outputBuffer;

// Carries the requirement on the member's type rather than on the member itself.
[require(metal)]
struct M
{
}

// The type parameter is intentionally unused: a generic parent is what keeps
// the reference from folding to a DirectDeclRef.
struct S<T>
{
    [require(spirv)]
    static uint SF() { return 1; }

    static M field;
}

// Control: the same members without a generic parent.
struct N
{
    [require(spirv)]
    static uint SF() { return 1; }

    static M field;
}

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain()
{
#ifdef TEST_GENERIC_FUNCTION
    // CHECK: error 36107
    outputBuffer[0] = S<float>.SF();
#endif
#ifdef TEST_NON_GENERIC_FUNCTION
    // CHECK: error 36107
    outputBuffer[0] = N.SF();
#endif
#ifdef TEST_TYPE_NON_GENERIC
    // CHECK: error 36107
    M a = N.field;
#endif
#ifdef TEST_TYPE_GENERIC
    // CHECK: error 36107
    M b = S<float>.field;
#endif
}
