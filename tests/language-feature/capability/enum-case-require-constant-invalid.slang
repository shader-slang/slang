//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage compute -DOPERAND_REQUIREMENT=1
// non-exhaustive: capability provenance also emits declaration/use notes.

enum TargetValue
{
    [require(hlsl)]
    hlslOnly = 0,
    portable = 1,
}

[require(hlsl)]
static const int hlslTag = 1;

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
//CHECK: entrypoint 'main' uses features that are not available
{
#if defined(OPERAND_REQUIREMENT)
    // Matching an unrestricted case must not lose the operand's requirement.
    outputBuffer[0] = int(TargetValue(hlslTag));
#else
    // A numeric spelling must check the same requirement as TargetValue.hlslOnly.
    outputBuffer[0] = int(TargetValue(0));
#endif
}
