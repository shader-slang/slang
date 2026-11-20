//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry hlslMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=SIZE,non-exhaustive):-target spirv -entry sizeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=ALIGN,non-exhaustive):-target spirv -entry alignMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=COUNT,non-exhaustive):-target spirv -entry countMain -stage compute
// non-exhaustive: capability errors include provenance notes.

enum Mode
{
    [require(hlsl)] hlslMode = 0,
}

struct Holder<let mode : Mode>
{
    int payload;
}

int readInt<let value : int>() { return value; }

RWStructuredBuffer<int> outputBuffer;

// HLSL: void hlslMain
[numthreads(1, 1, 1)]
void hlslMain()
{
    outputBuffer[0] = readInt<sizeof(Holder<Mode.hlslMode>)>();
    outputBuffer[1] = readInt<alignof(Holder<Mode.hlslMode>)>();
    outputBuffer[2] = readInt<countof(Tuple<Holder<Mode.hlslMode>>) >();
}

[numthreads(1, 1, 1)]
void sizeMain()
//SIZE: uses features that are not available
{
    // Folding the layout query must preserve requirements from its source type.
    outputBuffer[0] = readInt<sizeof(Holder<Mode.hlslMode>)>();
}

[numthreads(1, 1, 1)]
void alignMain()
//ALIGN: uses features that are not available
{
    outputBuffer[0] = readInt<alignof(Holder<Mode.hlslMode>)>();
}

[numthreads(1, 1, 1)]
void countMain()
//COUNT: uses features that are not available
{
    outputBuffer[0] = readInt<countof(Tuple<Holder<Mode.hlslMode>>) >();
}
