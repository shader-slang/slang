//TEST:COMPILE: tests/serialization/enum-case-require-module.slang -o tests/serialization/enum-case-require-module.slang-module
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry namedMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry castMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry defaultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=NAMED,non-exhaustive):-target spirv -entry namedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CAST,non-exhaustive):-target spirv -entry castMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=DEFAULT,non-exhaustive):-target spirv -entry defaultMain -stage compute
// non-exhaustive: constant casts warn and capability errors include provenance notes.

import "enum-case-require-module";

RWStructuredBuffer<int> outputBuffer;

// HLSL: RWStructuredBuffer
[numthreads(1, 1, 1)]
void namedMain()
//NAMED: entrypoint 'namedMain' uses features that are not available
{
    outputBuffer[0] = readValue<Mode.hlslMode>();
}

[numthreads(1, 1, 1)]
void castMain()
//CAST: entrypoint 'castMain' uses features that are not available
{
    outputBuffer[0] = readValue<Mode(0)>();
}

[numthreads(1, 1, 1)]
void defaultMain()
//DEFAULT: entrypoint 'defaultMain' uses features that are not available
{
    outputBuffer[0] = readDefault();
}
