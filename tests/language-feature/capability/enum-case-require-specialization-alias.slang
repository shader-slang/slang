//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry hlslMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry spirvMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry plainMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=UNAVAILABLE,non-exhaustive):-target spirv -entry unavailableMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=UNMATCHED,non-exhaustive):-target hlsl -entry unmatchedMain -stage compute
// non-exhaustive: integer conversions warn before specialization and errors include notes.

enum Mode
{
    [require(hlsl)] hlslMode = 0,
    [require(spirv)] spirvMode = 1,
}

struct IntHolder<let value : int>
{
    int payload;
}

typealias Alias<let tag : int> = IntHolder<int(Mode(tag)) * 0>;

int readAlias<let tag : int>()
{
    Alias<tag> value = { 7 };
    return value.payload;
}

RWStructuredBuffer<int> outputBuffer;

// HLSL: void hlslMain
[numthreads(1, 1, 1)]
void hlslMain()
{
    outputBuffer[0] = readAlias<0>();
}

// SPIRV: OpEntryPoint GLCompute
[numthreads(1, 1, 1)]
void spirvMain()
{
    outputBuffer[0] = readAlias<1>();
}

[numthreads(1, 1, 1)]
void plainMain()
{
    // Alias has the same canonical type, but its unused cast contributes no requirement here.
    IntHolder<0> value = { 7 };
    outputBuffer[0] = value.payload;
}

[numthreads(1, 1, 1)]
void unavailableMain()
//UNAVAILABLE: enum conversion selects an unavailable case
{
    outputBuffer[0] = readAlias<0>();
}

[numthreads(1, 1, 1)]
void unmatchedMain()
//UNMATCHED: enum conversion has no matching case after specialization
{
    outputBuffer[0] = readAlias<42>();
}
