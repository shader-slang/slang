//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry hlslMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry spirvMain -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry inferredMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=HLSL_ONLY,non-exhaustive):-target spirv -entry hlslMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=SPIRV_ONLY,non-exhaustive):-target hlsl -entry spirvMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=INFERRED,non-exhaustive):-target spirv -entry inferredMain -stage compute
// non-exhaustive: capability errors also include declaration/use notes.

enum Mode
{
    [require(hlsl)] A = 0,
    [require(spirv)] B = 1,
    // Equal numeric values do not make these declarations share requirements.
    [require(spirv)] sameValueAsA = 0,
    derived = int(A) + 2,
    // An implicit numeric initializer does not reference the preceding case.
    portable,
}

int consume(Mode value)
{
    Mode copy = value;
    return int(copy);
}

int readDerived()
{
    // Both this helper and the derived case infer A's requirement.
    return int(Mode.derived);
}

RWStructuredBuffer<int> outputBuffer;

// HLSL: RWStructuredBuffer
[numthreads(1, 1, 1)]
void hlslMain()
//HLSL_ONLY: entrypoint 'hlslMain' uses features that are not available
{
    outputBuffer[0] = consume(Mode.A);
    // Computing B's numeric value does not reference B.
    outputBuffer[1] = consume(Mode(int(Mode.A) + 1));
}

// SPIRV: OpEntryPoint GLCompute
[numthreads(1, 1, 1)]
void spirvMain()
//SPIRV_ONLY: entrypoint 'spirvMain' uses features that are not available
{
    outputBuffer[0] = consume(Mode.B);
    outputBuffer[1] = consume(Mode.sameValueAsA);
    outputBuffer[2] = consume(Mode.portable);
}

[numthreads(1, 1, 1)]
void inferredMain()
//INFERRED: entrypoint 'inferredMain' uses features that are not available
{
    outputBuffer[0] = readDerived();
}
