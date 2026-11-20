//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage compute -DCONSTANT_CAST=1
// non-exhaustive: capability provenance also emits declaration/use notes.

// Referencing the enum before its definition must still check the selected case.
int forwardReference()
{
#if defined(CONSTANT_CAST)
    return int(TargetValue(1));
#else
    return int(TargetValue.hlslOnly);
#endif
}

enum TargetValue
{
    [require(hlsl)]
    hlslOnly = 1,
    [require(spirv)]
    spirvOnly = 2,
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
//CHECK: entrypoint 'main' uses features that are not available
{
    outputBuffer[0] = forwardReference();
}
