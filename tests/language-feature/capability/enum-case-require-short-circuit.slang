//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry hlslMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FOLDED_SELECT,non-exhaustive):-target spirv -entry foldedSelectMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=RUNTIME_SELECT,non-exhaustive):-target spirv -entry runtimeSelectMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FOLDED_AND,non-exhaustive):-target spirv -entry foldedAndMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=RUNTIME_AND,non-exhaustive):-target spirv -entry runtimeAndMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FOLDED_OR,non-exhaustive):-target spirv -entry foldedOrMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=RUNTIME_OR,non-exhaustive):-target spirv -entry runtimeOrMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FOLDED_CONST,non-exhaustive):-target spirv -entry foldedConstMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=RUNTIME_CONST,non-exhaustive):-target spirv -entry runtimeConstMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FOLDED_MEMBER,non-exhaustive):-target spirv -entry foldedMemberMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=RUNTIME_MEMBER,non-exhaustive):-target spirv -entry runtimeMemberMain -stage compute
// non-exhaustive: capability errors include provenance notes.

enum Mode
{
    [require(hlsl)] hlslMode = 0,
}

static const int copiedCase = int(Mode.hlslMode);

struct Globals
{
    static const int copiedCase = int(Mode.hlslMode);
}

int readInt<let value : int>() { return value; }
RWStructuredBuffer<int> outputBuffer;

// HLSL: void hlslMain
[numthreads(1, 1, 1)]
void hlslMain()
{
    outputBuffer[0] = readInt<true ? 1 : int(Mode.hlslMode)>();
    outputBuffer[1] = readInt<int(false && bool(Mode.hlslMode))>();
    outputBuffer[2] = readInt<int(true || bool(Mode.hlslMode))>();
    outputBuffer[3] = readInt<true ? 1 : copiedCase>();
    outputBuffer[4] = readInt<true ? 1 : Globals.copiedCase>();
}

[numthreads(1, 1, 1)]
void foldedSelectMain()
//FOLDED_SELECT: entrypoint 'foldedSelectMain' uses features that are not available
{
    // Source case requirements survive even when numeric folding does not evaluate the arm.
    outputBuffer[0] = readInt<true ? 1 : int(Mode.hlslMode)>();
}

[numthreads(1, 1, 1)]
void runtimeSelectMain()
//RUNTIME_SELECT: entrypoint 'runtimeSelectMain' uses features that are not available
{
    outputBuffer[0] = true ? 1 : int(Mode.hlslMode);
}

[numthreads(1, 1, 1)]
void foldedAndMain()
//FOLDED_AND: entrypoint 'foldedAndMain' uses features that are not available
{
    outputBuffer[0] = readInt<int(false && bool(Mode.hlslMode))>();
}

[numthreads(1, 1, 1)]
void runtimeAndMain()
//RUNTIME_AND: entrypoint 'runtimeAndMain' uses features that are not available
{
    outputBuffer[0] = int(false && bool(Mode.hlslMode));
}

[numthreads(1, 1, 1)]
void foldedOrMain()
//FOLDED_OR: entrypoint 'foldedOrMain' uses features that are not available
{
    outputBuffer[0] = readInt<int(true || bool(Mode.hlslMode))>();
}

[numthreads(1, 1, 1)]
void runtimeOrMain()
//RUNTIME_OR: entrypoint 'runtimeOrMain' uses features that are not available
{
    outputBuffer[0] = int(true || bool(Mode.hlslMode));
}

[numthreads(1, 1, 1)]
void foldedConstMain()
//FOLDED_CONST: entrypoint 'foldedConstMain' uses features that are not available
{
    outputBuffer[0] = readInt<true ? 1 : copiedCase>();
}

[numthreads(1, 1, 1)]
void runtimeConstMain()
//RUNTIME_CONST: entrypoint 'runtimeConstMain' uses features that are not available
{
    outputBuffer[0] = true ? 1 : copiedCase;
}

[numthreads(1, 1, 1)]
void foldedMemberMain()
//FOLDED_MEMBER: entrypoint 'foldedMemberMain' uses features that are not available
{
    outputBuffer[0] = readInt<true ? 1 : Globals.copiedCase>();
}

[numthreads(1, 1, 1)]
void runtimeMemberMain()
//RUNTIME_MEMBER: entrypoint 'runtimeMemberMain' uses features that are not available
{
    outputBuffer[0] = true ? 1 : Globals.copiedCase;
}
