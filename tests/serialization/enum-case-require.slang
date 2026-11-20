//TEST:COMPILE: tests/serialization/enum-case-require-module.slang -module-name enum_case_require_cached -o tests/serialization/enum_case_require_cached.slang-module
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry namedMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry castMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry defaultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=NAMED,non-exhaustive):-target spirv -entry namedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CAST,non-exhaustive):-target spirv -entry castMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=DEFAULT,non-exhaustive):-target spirv -entry defaultMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry typeDefaultMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry typeDefaultMain -stage compute -DSOURCE_IMPORT=1
//DIAGNOSTIC_TEST:SIMPLE(diag=TYPE_DEFAULT,non-exhaustive):-target spirv -entry typeDefaultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=TYPE_DEFAULT,non-exhaustive):-target spirv -entry typeDefaultMain -stage compute -DSOURCE_IMPORT=1
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry symbolicMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry cancelledMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry overrideMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=SYMBOLIC,non-exhaustive):-target spirv -entry symbolicMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CANCELLED,non-exhaustive):-target spirv -entry cancelledMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=UNMATCHED,non-exhaustive):-target hlsl -entry unmatchedMain -stage compute
// non-exhaustive: constant casts warn and capability errors include provenance notes.

#if defined(SOURCE_IMPORT)
import "enum-case-require-module";
#else
// There is no source file with this module name, so these tests require deserialization.
import enum_case_require_cached;
#endif

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

[numthreads(1, 1, 1)]
void typeDefaultMain()
//TYPE_DEFAULT: uses features that are not available
{
    outputBuffer[0] = readTypeDefault();
}

[numthreads(1, 1, 1)]
void symbolicMain()
//SYMBOLIC: enum conversion selects an unavailable case
{
    outputBuffer[0] = readSymbolic<-1>();
}

[numthreads(1, 1, 1)]
void cancelledMain()
//CANCELLED: enum conversion selects an unavailable case
{
    outputBuffer[0] = readSymbolicCancelled<0>();
}

[numthreads(1, 1, 1)]
void unmatchedMain()
//UNMATCHED: enum conversion has no matching case after specialization
{
    outputBuffer[0] = readSymbolic<41>();
}

// SPIRV: OpEntryPoint GLCompute
[numthreads(1, 1, 1)]
void overrideMain()
{
    outputBuffer[0] = readTypeDefault<Holder<Mode.spirvMode>>();
    outputBuffer[1] = readSymbolic<0>();
    outputBuffer[2] = readSymbolicCancelled<1>();
}
