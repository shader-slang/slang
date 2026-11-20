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
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry wrapperMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry nativeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=WRAPPER,non-exhaustive):-target spirv -entry wrapperMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=WRAPPER_UNMATCHED,non-exhaustive):-target hlsl -entry wrapperUnmatchedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=NATIVE,non-exhaustive):-target spirv -entry nativeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=NATIVE_UNMATCHED,non-exhaustive):-target hlsl -entry nativeUnmatchedMain -stage compute
// non-exhaustive: constant casts warn and capability errors include provenance notes.

#if defined(SOURCE_IMPORT)
import "enum-case-require-module";
#else
// There is no source file with this module name, so these tests require deserialization.
import enum_case_require_cached;
#endif

RWStructuredBuffer<int> outputBuffer;

int readImportedWrapper<let tag : int>()
{
    // The selected declaration owns the cast, even though this consumer never reads its field.
    Wrapper<tag> value;
    value.payload = 7;
    return value.payload;
}

int readImportedNativeWrapper<let tag : int>()
{
    // The native representation's deferred validation must survive module serialization too.
    NativeWrapper<tag> value;
    return 9;
}

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
    outputBuffer[3] = readImportedWrapper<1>();
    outputBuffer[4] = readImportedNativeWrapper<1>();
}

[numthreads(1, 1, 1)]
void wrapperMain()
//WRAPPER: enum conversion selects an unavailable case
{
    outputBuffer[0] = readImportedWrapper<0>();
}

[numthreads(1, 1, 1)]
void wrapperUnmatchedMain()
//WRAPPER_UNMATCHED: enum conversion has no matching case after specialization
{
    outputBuffer[0] = readImportedWrapper<42>();
}

[numthreads(1, 1, 1)]
void nativeMain()
//NATIVE: enum conversion selects an unavailable case
{
    outputBuffer[0] = readImportedNativeWrapper<0>();
}

[numthreads(1, 1, 1)]
void nativeUnmatchedMain()
//NATIVE_UNMATCHED: enum conversion has no matching case after specialization
{
    outputBuffer[0] = readImportedNativeWrapper<42>();
}
