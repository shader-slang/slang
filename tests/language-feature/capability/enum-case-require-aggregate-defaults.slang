//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry nestedMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry arrayMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry symbolicArrayMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry overrideMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry genericZeroMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTED,non-exhaustive):-target spirv -entry nestedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=ARRAY,non-exhaustive):-target spirv -entry arrayMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=SYMBOLIC_ARRAY,non-exhaustive):-target spirv -entry symbolicArrayMain -stage compute
// non-exhaustive: capability errors include provenance notes.

enum Mode
{
    [require(hlsl)] hlslMode = 1,
    [require(spirv)] spirvMode = 2,
}

struct FieldDefault
{
    Mode mode = Mode.hlslMode;
}

struct Wrapper
{
    int first;
    FieldDefault value;
}

struct GenericWrapper<T>
{
    int first;
    T value;
}

int readArray<let N : int>()
{
    FieldDefault values[N] = {};
    return int(values[0].mode);
}

RWStructuredBuffer<int> outputBuffer;

// HLSL: RWStructuredBuffer
[numthreads(1, 1, 1)]
void nestedMain()
//NESTED: entrypoint 'nestedMain' uses features that are not available
{
    // The omitted concrete field selects its nested field default.
    Wrapper value = { 0 };
    outputBuffer[0] = int(value.value.mode);
}

[numthreads(1, 1, 1)]
void arrayMain()
//ARRAY: entrypoint 'arrayMain' uses features that are not available
{
    FieldDefault values[2] = {};
    outputBuffer[0] = int(values[1].mode);
}

[numthreads(1, 1, 1)]
void symbolicArrayMain()
//SYMBOLIC_ARRAY: entrypoint 'symbolicArrayMain' uses features that are not available
{
    outputBuffer[0] = readArray<2>();
}

// SPIRV: OpEntryPoint GLCompute
[numthreads(1, 1, 1)]
void overrideMain()
{
    // Explicitly supplied fields do not use the HLSL default.
    Wrapper value = { 0, { Mode.spirvMode } };
    FieldDefault values[2] = { { Mode.spirvMode }, { Mode.spirvMode } };
    outputBuffer[0] = int(value.value.mode) + int(values[1].mode);
}

[numthreads(1, 1, 1)]
void genericZeroMain()
{
    // The synthesized generic constructor zero-initializes T. It does not select
    // FieldDefault's field initializer, so no named case requirement is introduced.
    GenericWrapper<FieldDefault> value = { 0 };
    outputBuffer[0] = int(value.value.mode);
}
