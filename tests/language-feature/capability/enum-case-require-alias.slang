//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage compute
// non-exhaustive: capability provenance also emits declaration/use notes.

enum TargetValue
{
    [require(hlsl)]
    hlslOnly = 1,
    aliasOfHlsl = hlslOnly,
    // An automatically assigned integer tag does not reference the previous case.
    portable,
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
//CHECK: entrypoint 'main' uses features that are not available
{
    outputBuffer[0] = int(TargetValue.aliasOfHlsl);
}
