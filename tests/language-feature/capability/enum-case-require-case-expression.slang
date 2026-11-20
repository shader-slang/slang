//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry portableMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage compute
// non-exhaustive: capability provenance also emits declaration/use notes.

enum TargetValue
{
    [require(hlsl)]
    hlslOnly = 1,
    derivedHlsl = int(hlslOnly) + 1,
    // An automatically assigned integer tag does not reference the previous case.
    portable,
}

RWStructuredBuffer<int> outputBuffer;

// HLSL: void main
[numthreads(1, 1, 1)]
void main()
//CHECK: entrypoint 'main' uses features that are not available
{
    outputBuffer[0] = int(TargetValue.derivedHlsl);
}

// SPIRV: OpEntryPoint GLCompute
[numthreads(1, 1, 1)]
void portableMain()
{
    outputBuffer[0] = int(TargetValue.portable);
}
