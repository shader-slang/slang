//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry hlslMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry spirvMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=UNSUPPORTED,non-exhaustive):-target spirv -entry unsupportedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CANCELLED,non-exhaustive):-target spirv -entry cancelledMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=NAMED_CANCELLED,non-exhaustive):-target spirv -entry namedCancelledMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=UNMATCHED,non-exhaustive):-target hlsl -entry unmatchedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC_CANCELLED,non-exhaustive):-target spirv -entry genericCancelledMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=HOLDER,non-exhaustive):-target spirv -entry holderMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC_UNMATCHED,non-exhaustive):-target hlsl -entry genericUnmatchedMain -stage compute
// non-exhaustive: integer conversions warn and capability errors include provenance notes.

enum Mode
{
    [require(hlsl)] hlslMode = 0,
    [require(spirv)] spirvMode = 1,
}

enum NarrowMode : uint8_t
{
    [require(hlsl)] zero = 0,
}

Mode fromTag<let tag : int>()
{
    return Mode(tag);
}

int readArithmetic<let tag : int>()
{
    return int(Mode(tag + 1));
}

int readCancelled<let tag : int>()
{
    // Simplification may remove the numerical result, but the selected case is still used.
    return int(Mode(tag)) * 0;
}

int readNarrow<let tag : int>()
{
    // Normalize the specialized value to the enum's tag type before selecting the case.
    return int(NarrowMode(tag));
}

struct IntegerHolder<let value : int>
{
    int payload;
}

int readInt<let value : int>() { return value; }

struct AliasHolder<let value : uint32_t>
{
    int payload;
}

int readAlias<let value : uint32_t>(AliasHolder<value> holder)
{
    return holder.payload;
}

int forwardAlias<let value : uint32_t>(AliasHolder<value> holder)
{
    // Generic substitution must preserve equality between uint32_t and its canonical uint.
    return readAlias<value>(holder);
}

int readGenericCancelled<let tag : int>()
{
    // Folding a generic argument to zero must retain the conversion's validation.
    return readInt<int(Mode(tag)) * 0>();
}

struct ModeHolder<let mode : Mode>
{
    int payload;
}

int readHolder<let tag : int>()
{
    // Using the specialized type selects the case even if its tag is not read at runtime.
    ModeHolder<Mode(tag)> value = { 5 };
    return value.payload;
}

[require(hlsl)]
int checkCanonicalEquality()
{
    // Capability provenance must not change canonical generic values or array extents.
    IntegerHolder<int(Mode.hlslMode) + 1> value = { 7 };
    IntegerHolder<1> canonical = value;
    int source[int(Mode.hlslMode) + 1] = { canonical.payload };
    int destination[1] = source;
    return destination[0];
}

int readNamedCancelled<let value : int>()
{
    // The named case requirement survives folding mixed symbolic arithmetic to zero.
    return readInt<(int(Mode.hlslMode) + value) * 0>();
}

RWStructuredBuffer<int> outputBuffer;

// HLSL: void hlslMain
[numthreads(1, 1, 1)]
void hlslMain()
{
    outputBuffer[0] = int(fromTag<0>());
    outputBuffer[1] = readArithmetic<-1>();
    outputBuffer[2] = readCancelled<0>();
    outputBuffer[3] = readNarrow<256>();
    outputBuffer[4] = checkCanonicalEquality();
    outputBuffer[5] = readNamedCancelled<3>();
    outputBuffer[6] = readGenericCancelled<0>();
    outputBuffer[7] = readHolder<0>();
    AliasHolder<1> aliasValue = { 4 };
    outputBuffer[8] = forwardAlias<1>(aliasValue);
}

// SPIRV: OpEntryPoint GLCompute
[numthreads(1, 1, 1)]
void spirvMain()
{
    outputBuffer[0] = int(fromTag<1>());
    outputBuffer[1] = readArithmetic<0>();
    outputBuffer[2] = readCancelled<1>();
    outputBuffer[3] = readGenericCancelled<1>();
    outputBuffer[4] = readHolder<1>();
}

[numthreads(1, 1, 1)]
void unsupportedMain()
//UNSUPPORTED: enum conversion selects an unavailable case
{
    outputBuffer[0] = readArithmetic<-1>();
}

[numthreads(1, 1, 1)]
void cancelledMain()
//CANCELLED: enum conversion selects an unavailable case
{
    outputBuffer[0] = readCancelled<0>();
}

[numthreads(1, 1, 1)]
void unmatchedMain()
//UNMATCHED: enum conversion has no matching case after specialization
{
    outputBuffer[0] = int(fromTag<42>());
}

[numthreads(1, 1, 1)]
void namedCancelledMain()
//NAMED_CANCELLED: uses features that are not available
{
    outputBuffer[0] = readNamedCancelled<3>();
}

[numthreads(1, 1, 1)]
void genericCancelledMain()
//GENERIC_CANCELLED: enum conversion selects an unavailable case
{
    outputBuffer[0] = readGenericCancelled<0>();
}

[numthreads(1, 1, 1)]
void holderMain()
//HOLDER: enum conversion selects an unavailable case
{
    outputBuffer[0] = readHolder<0>();
}

[numthreads(1, 1, 1)]
void genericUnmatchedMain()
//GENERIC_UNMATCHED: enum conversion has no matching case after specialization
{
    outputBuffer[0] = readGenericCancelled<42>();
}
