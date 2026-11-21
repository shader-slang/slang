//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry namedMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry castMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry typeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry staticMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry staticGenericMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry partialMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry defaultTypeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry defaultMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry bareTypeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry resultMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry aliasMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry modifiedTypeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry defaultFieldMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry defaultConstructorMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry defaultParameterMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry arrayMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry forwardStaticValueMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry forwardAliasMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry overrideMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=NAMED,non-exhaustive):-target spirv -entry namedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CAST,non-exhaustive):-target spirv -entry castMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=TYPE,non-exhaustive):-target spirv -entry typeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=STATIC,non-exhaustive):-target spirv -entry staticMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=STATIC_GENERIC,non-exhaustive):-target spirv -entry staticGenericMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PARTIAL,non-exhaustive):-target spirv -entry partialMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=DEFAULT_TYPE,non-exhaustive):-target spirv -entry defaultTypeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=DEFAULT,non-exhaustive):-target spirv -entry defaultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=BARE_TYPE,non-exhaustive):-target spirv -entry bareTypeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=RESULT,non-exhaustive):-target spirv -entry resultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=ALIAS,non-exhaustive):-target spirv -entry aliasMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=MODIFIED_TYPE,non-exhaustive):-target spirv -entry modifiedTypeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=DEFAULT_FIELD,non-exhaustive):-target spirv -entry defaultFieldMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=DEFAULT_CONSTRUCTOR,non-exhaustive):-target spirv -entry defaultConstructorMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=DEFAULT_PARAMETER,non-exhaustive):-target spirv -entry defaultParameterMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=ARRAY,non-exhaustive):-target spirv -entry arrayMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FORWARD_STATIC,non-exhaustive):-target spirv -entry forwardStaticValueMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FORWARD_ALIAS,non-exhaustive):-target spirv -entry forwardAliasMain -stage compute
// non-exhaustive: accepted constant casts warn, and capability errors include provenance notes.

enum Mode
{
    [require(hlsl)] hlslMode = 0,
    [require(spirv)] spirvMode = 1,
}

int readValue<let V : Mode>() { return int(V); }

int readPartial<let V : Mode, T>(T unused) { return int(V); }

int readDefault<let V : Mode = Mode.hlslMode>() { return int(V); }

int readParameter(Mode mode = Mode.hlslMode) { return int(mode); }

struct Holder<let V : Mode = Mode.hlslMode>
{
    int value;
    static int read() { return int(V); }
    static int readGeneric<T>() { return int(V); }
}

struct DefaultHolder<T, let V : Mode = Mode.hlslMode>
{
    T value;
}

Holder<Mode.hlslMode> makeHolder() { return { 0 }; }

typealias HlslHolder = Holder<Mode.hlslMode>;

int readModified(no_diff Holder<Mode.hlslMode> holder) { return holder.value; }

struct FieldDefault
{
    Mode value = Mode.hlslMode;
}

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
void typeMain()
//TYPE: entrypoint 'typeMain' uses features that are not available
{
    Holder<Mode.hlslMode> holder = { 0 };
    outputBuffer[0] = holder.value;
}

[numthreads(1, 1, 1)]
void staticMain()
//STATIC: entrypoint 'staticMain' uses features that are not available
{
    outputBuffer[0] = Holder<Mode.hlslMode>.read();
}

[numthreads(1, 1, 1)]
void staticGenericMain()
//STATIC_GENERIC: entrypoint 'staticGenericMain' uses features that are not available
{
    outputBuffer[0] = Holder<Mode.hlslMode>.readGeneric<int>();
}

[numthreads(1, 1, 1)]
void partialMain()
//PARTIAL: entrypoint 'partialMain' uses features that are not available
{
    outputBuffer[0] = readPartial<Mode.hlslMode>(0);
}

[numthreads(1, 1, 1)]
void defaultTypeMain()
//DEFAULT_TYPE: entrypoint 'defaultTypeMain' uses features that are not available
{
    DefaultHolder<int> holder = { 0 };
    outputBuffer[0] = holder.value;
}

[numthreads(1, 1, 1)]
void defaultMain()
//DEFAULT: entrypoint 'defaultMain' uses features that are not available
{
    outputBuffer[0] = readDefault();
}

[numthreads(1, 1, 1)]
void bareTypeMain()
//BARE_TYPE: entrypoint 'bareTypeMain' uses features that are not available
{
    Holder holder = { 0 };
    outputBuffer[0] = holder.value;
}

[numthreads(1, 1, 1)]
void resultMain()
//RESULT: entrypoint 'resultMain' uses features that are not available
{
    outputBuffer[0] = makeHolder().value;
}

[numthreads(1, 1, 1)]
void aliasMain()
//ALIAS: entrypoint 'aliasMain' uses features that are not available
{
    HlslHolder holder = { 0 };
    outputBuffer[0] = holder.value;
}

[numthreads(1, 1, 1)]
void modifiedTypeMain()
//MODIFIED_TYPE: entrypoint 'modifiedTypeMain' uses features that are not available
{
    outputBuffer[0] = readModified({ 0 });
}

[numthreads(1, 1, 1)]
void defaultFieldMain()
//DEFAULT_FIELD: entrypoint 'defaultFieldMain' uses features that are not available
{
    FieldDefault value = {};
    outputBuffer[0] = int(value.value);
}

[numthreads(1, 1, 1)]
void defaultConstructorMain()
//DEFAULT_CONSTRUCTOR: entrypoint 'defaultConstructorMain' uses features that are not available
{
    FieldDefault value = FieldDefault();
    outputBuffer[0] = int(value.value);
}

[numthreads(1, 1, 1)]
void defaultParameterMain()
//DEFAULT_PARAMETER: entrypoint 'defaultParameterMain' uses features that are not available
{
    outputBuffer[0] = readParameter();
}

[numthreads(1, 1, 1)]
void arrayMain()
//ARRAY: entrypoint 'arrayMain' uses features that are not available
{
    float values[int(Mode.hlslMode) + 1] = { 1.0 };
    outputBuffer[0] = int(values[0]);
}

[numthreads(1, 1, 1)]
void forwardStaticValueMain()
//FORWARD_STATIC: entrypoint 'forwardStaticValueMain' uses features that are not available
{
    outputBuffer[0] = Later.value;
}

[numthreads(1, 1, 1)]
void forwardAliasMain()
//FORWARD_ALIAS: entrypoint 'forwardAliasMain' uses features that are not available
{
    LaterHolder holder = { 0 };
    outputBuffer[0] = holder.value;
}

struct Later
{
    static const int value = int(Mode.hlslMode);
}

typealias LaterHolder = Holder<Mode.hlslMode>;

// SPIRV: OpEntryPoint GLCompute
[numthreads(1, 1, 1)]
void overrideMain()
{
    // An unused HLSL default must not restrict an explicit SPIR-V specialization.
    outputBuffer[0] = readDefault<Mode.spirvMode>();
    Holder<Mode.spirvMode> holder = { 0 };
    outputBuffer[1] = holder.value;
    FieldDefault value = { Mode.spirvMode };
    outputBuffer[2] = int(value.value);
    outputBuffer[3] = readParameter(Mode.spirvMode);
    FieldDefault constructed = FieldDefault(Mode.spirvMode);
    outputBuffer[4] = int(constructed.value);
}
