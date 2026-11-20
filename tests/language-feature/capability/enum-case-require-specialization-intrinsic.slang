//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry hlslMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry spirvMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=UNMATCHED,non-exhaustive):-target hlsl -entry unmatchedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=UNAVAILABLE,non-exhaustive):-target spirv -entry unavailableMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTED_UNMATCHED,non-exhaustive):-target hlsl -entry nestedUnmatchedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTED_UNAVAILABLE,non-exhaustive):-target spirv -entry nestedUnavailableMain -stage compute
// non-exhaustive: integer conversions warn before specialization and errors include notes.

enum Mode
{
    [require(hlsl)] hlslMode = 0,
    [require(spirv)] spirvMode = 1,
}

struct Holder<let mode : Mode>
{
    int value;
}

// The target's native representation does not erase checks from the selected Slang type.
__target_intrinsic(hlsl, "int")
struct NativeWrapper<let tag : int>
{
    Holder<Mode(tag)> field;
}

__target_intrinsic(hlsl, "int")
struct NestedNativeWrapper<let tag : int>
{
    NativeWrapper<tag> field;
}

int readNative<let tag : int>()
{
    NativeWrapper<tag> value;
    return 7;
}

int readNestedNative<let tag : int>()
{
    NestedNativeWrapper<tag> value;
    return 9;
}

int unusedNative()
{
    // An unreachable function must not select this specialization's unmatched tag.
    NativeWrapper<42> value;
    return 0;
}

RWStructuredBuffer<int> outputBuffer;

// HLSL: void hlslMain
[numthreads(1, 1, 1)]
void hlslMain()
{
    outputBuffer[0] = readNative<0>();
    outputBuffer[1] = readNestedNative<0>();
    if (false)
    {
        // Deferred checks in eliminated branches do not execute.
        NativeWrapper<42> unused;
    }
}

// SPIRV: OpEntryPoint GLCompute
[numthreads(1, 1, 1)]
void spirvMain()
{
    outputBuffer[0] = readNative<1>();
    outputBuffer[1] = readNestedNative<1>();
}

[numthreads(1, 1, 1)]
void unmatchedMain()
//UNMATCHED: enum conversion has no matching case after specialization
{
    outputBuffer[0] = readNative<42>();
}

[numthreads(1, 1, 1)]
void unavailableMain()
//UNAVAILABLE: enum conversion selects an unavailable case
{
    outputBuffer[0] = readNative<0>();
}

[numthreads(1, 1, 1)]
void nestedUnmatchedMain()
//NESTED_UNMATCHED: enum conversion has no matching case after specialization
{
    outputBuffer[0] = readNestedNative<42>();
}

[numthreads(1, 1, 1)]
void nestedUnavailableMain()
//NESTED_UNAVAILABLE: enum conversion selects an unavailable case
{
    outputBuffer[0] = readNestedNative<0>();
}
