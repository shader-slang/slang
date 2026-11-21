//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage compute -DCONSTANT_CAST=1
// non-exhaustive: capability provenance also emits declaration/use notes, and the cast warns.

enum TargetValue
{
    [require(hlsl)]
    hlslOnly = 0,
    [require(spirv)]
    spirvOnly = 1,
}

#if defined(CONSTANT_CAST)
static const TargetValue selected = TargetValue(0);
#else
static const TargetValue selected = TargetValue.hlslOnly;
#endif

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
//CHECK: entrypoint 'main' uses features that are not available
{
    outputBuffer[0] = int(selected);
}
