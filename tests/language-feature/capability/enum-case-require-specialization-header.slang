//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry hlslMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry spirvMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FIELD,non-exhaustive):-target spirv -entry fieldMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=RETURN,non-exhaustive):-target spirv -entry returnMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FIELD_UNMATCHED,non-exhaustive):-target hlsl -entry fieldUnmatchedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=RETURN_UNMATCHED,non-exhaustive):-target hlsl -entry returnUnmatchedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=UNRELATED,non-exhaustive):-target spirv -entry unrelatedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=SYMBOLIC_RETURN,non-exhaustive):-target spirv -entry symbolicReturnMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=UNRELATED_UNMATCHED,non-exhaustive):-target hlsl -entry unrelatedUnmatchedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=SYMBOLIC_RETURN_UNMATCHED,non-exhaustive):-target hlsl -entry symbolicReturnUnmatchedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=TYPE_ARGUMENT,non-exhaustive):-target spirv -entry typeArgumentMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=TYPE_ARGUMENT_UNMATCHED,non-exhaustive):-target hlsl -entry typeArgumentUnmatchedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=SIZE,non-exhaustive):-target spirv -entry sizeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=SIZE_UNMATCHED,non-exhaustive):-target hlsl -entry sizeUnmatchedMain -stage compute
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

struct Wrapper<let tag : int>
{
    // Enum conversions in declaration headers need the enum's checked tag type too.
    Holder<Mode(tag)> field;
    int unrelated;
}

Holder<Mode(tag)> makeHolder<let tag : int>()
{
    return { 7 };
}

int readUnrelated<let tag : int>()
{
    // The whole specialized type is used, even when only its unrelated field is accessed.
    Wrapper<tag> value;
    value.unrelated = 7;
    return value.unrelated;
}

int readReturn<let tag : int>()
{
    return makeHolder<tag>().value;
}

void unused<T>() {}
int readInt<let value : int>() { return value; }

int readTypeArgument<let tag : int>()
{
    // A selected type argument still uses the type when the callee's body does not.
    unused<Wrapper<tag>>();
    return 7;
}

int readSize<let tag : int>()
{
    return readInt<sizeof(Wrapper<tag>)>();
}

// An unused generic declaration does not select its default argument or either field tag.
struct UnusedWrapper<let tag : int = 0>
{
    Holder<Mode(tag)> unavailableField;
    Holder<Mode(tag + 42)> unmatchedField;
}

RWStructuredBuffer<int> outputBuffer;

// HLSL: void hlslMain
[numthreads(1, 1, 1)]
void hlslMain()
{
    Wrapper<0> value;
    value.field.value = 7;
    outputBuffer[0] = value.field.value;
    outputBuffer[1] = makeHolder<0>().value;
    outputBuffer[2] = readUnrelated<0>();
    outputBuffer[3] = readReturn<0>();
    outputBuffer[4] = readTypeArgument<0>();
    outputBuffer[5] = readSize<0>();
}

// SPIRV: OpEntryPoint GLCompute
[numthreads(1, 1, 1)]
void spirvMain()
{
    Wrapper<1> value;
    value.field.value = 7;
    outputBuffer[0] = value.field.value;
    outputBuffer[1] = makeHolder<1>().value;
    outputBuffer[2] = readUnrelated<1>();
    outputBuffer[3] = readReturn<1>();
    outputBuffer[4] = readTypeArgument<1>();
    outputBuffer[5] = readSize<1>();
}

[numthreads(1, 1, 1)]
void fieldMain()
//FIELD: entrypoint 'fieldMain' uses features that are not available
{
    Wrapper<0> value;
    value.field.value = 7;
    outputBuffer[0] = value.field.value;
}

[numthreads(1, 1, 1)]
void returnMain()
//RETURN: entrypoint 'returnMain' uses features that are not available
{
    outputBuffer[0] = makeHolder<0>().value;
}

[numthreads(1, 1, 1)]
void fieldUnmatchedMain()
//FIELD_UNMATCHED: enum conversion has no matching case after specialization
{
    Wrapper<42> value;
    value.field.value = 7;
    outputBuffer[0] = value.field.value;
}

[numthreads(1, 1, 1)]
void returnUnmatchedMain()
//RETURN_UNMATCHED: enum conversion has no matching case after specialization
{
    outputBuffer[0] = makeHolder<42>().value;
}

[numthreads(1, 1, 1)]
void unrelatedMain()
//UNRELATED: enum conversion selects an unavailable case
{
    outputBuffer[0] = readUnrelated<0>();
}

[numthreads(1, 1, 1)]
void symbolicReturnMain()
//SYMBOLIC_RETURN: enum conversion selects an unavailable case
{
    outputBuffer[0] = readReturn<0>();
}

[numthreads(1, 1, 1)]
void unrelatedUnmatchedMain()
//UNRELATED_UNMATCHED: enum conversion has no matching case after specialization
{
    outputBuffer[0] = readUnrelated<42>();
}

[numthreads(1, 1, 1)]
void symbolicReturnUnmatchedMain()
//SYMBOLIC_RETURN_UNMATCHED: enum conversion has no matching case after specialization
{
    outputBuffer[0] = readReturn<42>();
}

[numthreads(1, 1, 1)]
void typeArgumentMain()
//TYPE_ARGUMENT: enum conversion selects an unavailable case
{
    outputBuffer[0] = readTypeArgument<0>();
}

[numthreads(1, 1, 1)]
void typeArgumentUnmatchedMain()
//TYPE_ARGUMENT_UNMATCHED: enum conversion has no matching case after specialization
{
    outputBuffer[0] = readTypeArgument<42>();
}

[numthreads(1, 1, 1)]
void sizeMain()
//SIZE: enum conversion selects an unavailable case
{
    outputBuffer[0] = readSize<0>();
}

[numthreads(1, 1, 1)]
void sizeUnmatchedMain()
//SIZE_UNMATCHED: enum conversion has no matching case after specialization
{
    outputBuffer[0] = readSize<42>();
}
