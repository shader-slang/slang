// A [require] on a static member must be enforced when the member is reached
// through a generic type, matching the non-generic spelling — for the member
// declaration's own requirements (functions and static generic methods) and
// for requirements carried by the member's type. The generic spellings stay
// unfolded because ASTBuilder::getMemberDeclRef folds only DirectDeclRef
// parents; a generic parent keeps a MemberDeclRef whose inner member decl the
// reference walk never dispatches.
// Regression test for https://github.com/shader-slang/slang/issues/14430

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage compute -entry computeMain -DTEST_GENERIC_FUNCTION
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage compute -entry computeMain -DTEST_NON_GENERIC_FUNCTION
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage compute -entry computeMain -DTEST_GENERIC_METHOD
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage compute -entry computeMain -DTEST_TYPE_NON_GENERIC
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage compute -entry computeMain -DTEST_TYPE_GENERIC
// The satisfied-capability direction must stay clean too.
//TEST:SIMPLE: -target spirv -stage compute -entry computeMain -DTEST_GENERIC_FUNCTION -o capability-static-member-generic.spvasm

RWStructuredBuffer<float> outputBuffer;

// Carries the requirement on the member's type rather than on the member
// itself. The reference spellings below reach `M` only through the field, so
// the diagnostic can come from no other route.
[require(metal)]
struct M
{
    uint x;
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

// A static generic method on a non-generic type — the same MemberDeclRef
// shape with the generic on the method instead of the parent.
struct P
{
    [require(spirv)]
    static uint GM<T>() { return 1; }
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
#ifdef TEST_GENERIC_METHOD
    // CHECK: error 36107
    outputBuffer[0] = P.GM<float>();
#endif
#ifdef TEST_TYPE_NON_GENERIC
    // CHECK: error 36107
    outputBuffer[0] = N.field.x;
#endif
#ifdef TEST_TYPE_GENERIC
    // CHECK: error 36107
    outputBuffer[0] = S<float>.field.x;
#endif
#ifdef TEST_GENERIC_FUNCTION
    // spirv satisfies SF's [require]; the compile must be clean.
#endif
}
