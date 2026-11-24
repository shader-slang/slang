//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry hlslMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry parameterOverrideMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry genericOverrideMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry unusedDefaultsMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PARAMETER_DEFAULT,non-exhaustive):-target spirv -entry parameterDefaultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC_DEFAULT,non-exhaustive):-target spirv -entry genericDefaultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=TYPE_DEFAULT,non-exhaustive):-target spirv -entry typeDefaultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=HOLDER_DEFAULT,non-exhaustive):-target spirv -entry holderDefaultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PARTIAL_PARAMETER,non-exhaustive):-target spirv -entry partialParameterMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PARTIAL_GENERIC,non-exhaustive):-target spirv -entry partialGenericMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FOLDED_PARAMETER,non-exhaustive):-target spirv -entry foldedParameterMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FOLDED_GENERIC,non-exhaustive):-target spirv -entry foldedGenericMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FORWARDED_DEFAULT,non-exhaustive):-target spirv -entry forwardedDefaultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=BODY_OVERRIDE,non-exhaustive):-target spirv -entry bodyOverrideMain -stage compute
// non-exhaustive: capability errors also include declaration/use notes.

enum Mode
{
    [require(hlsl)] A = 0,
}

// A default contributes its requirement only when the caller uses it.
int use(Mode mode = Mode.A) { return int(mode); }
int readDefault<let V : Mode = Mode.A>() { return 7; }

struct Holder<let V : Mode = Mode.A>
{
    int payload;
}

int readTypeDefault<T = Holder<Mode.A>>() { return 7; }
int inferDefault<let V : Mode = Mode.A>(Holder<V> value) { return value.payload; }
int inferTypeDefault<T = Holder<Mode.A>>(T value) { return 7; }

int usePartial(Mode mode = Mode.A, int extra = 0) { return int(mode) + extra; }
int useTrailing(int value = 0, Mode mode = Mode.A) { return value + int(mode); }
int readPartial<let V : Mode = Mode.A, let N : int = 0>() { return N; }
int readTrailing<let N : int = 0, let V : Mode = Mode.A>() { return N; }

// Folding these defaults to zero must preserve the requirement of their source reference.
int useFolded(int value = int(Mode.A) * 0) { return value; }
int readFolded<let V : int = int(Mode.A) * 0>() { return V; }

int forward(Mode mode = Mode.A) { return use(mode); }

int select(int value) { return value; }
int select(float value, Mode mode = Mode.A) { return int(value) + int(mode); }

int selectGeneric(int value) { return value; }
int selectGeneric<let V : Mode = Mode.A>(float value) { return int(value); }

int useBody(Mode mode = Mode.A) { return int(mode) + int(Mode.A); }

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void parameterDefaultMain()
//PARAMETER_DEFAULT: entrypoint 'parameterDefaultMain' uses features that are not available
{
    outputBuffer[0] = use();
}

[numthreads(1, 1, 1)]
void parameterOverrideMain()
{
    // The numeric override has no case requirement of its own.
    outputBuffer[0] = use(Mode(1));
}

[numthreads(1, 1, 1)]
void genericDefaultMain()
//GENERIC_DEFAULT: entrypoint 'genericDefaultMain' uses features that are not available
{
    outputBuffer[0] = readDefault();
}

[numthreads(1, 1, 1)]
void genericOverrideMain()
{
    outputBuffer[0] = readDefault<Mode(1)>();
}

// SPIRV: OpEntryPoint GLCompute
[numthreads(1, 1, 1)]
void unusedDefaultsMain()
{
    Holder<Mode(1)> value = { 7 };
    outputBuffer[0] = readTypeDefault<int>();
    // Both inferred arguments override a restricted default.
    outputBuffer[1] = inferDefault(value) + inferTypeDefault(7);
    // Supplying only the restricted parameter leaves an unrestricted default in use.
    outputBuffer[2] = usePartial(Mode(1)) + readPartial<Mode(1)>();
    outputBuffer[3] = useTrailing(1, Mode(1)) + readTrailing<1, Mode(1)>();
    outputBuffer[4] = useFolded(0) + readFolded<0>();
    outputBuffer[5] = forward(Mode(1));
    // The float overload's default is not used when the int overload wins.
    outputBuffer[6] = select(1);
    // Equal numeric values do not mean that inference selected the restricted default.
    Holder<Mode(0)> sameValue = { 7 };
    outputBuffer[7] = inferDefault(sameValue);
    // A generic default belongs only to the overload that is selected.
    outputBuffer[8] = selectGeneric(1);
}

[numthreads(1, 1, 1)]
void typeDefaultMain()
//TYPE_DEFAULT: entrypoint 'typeDefaultMain' uses features that are not available
{
    outputBuffer[0] = readTypeDefault();
}

[numthreads(1, 1, 1)]
void holderDefaultMain()
//HOLDER_DEFAULT: entrypoint 'holderDefaultMain' uses features that are not available
{
    Holder<> value = { 7 };
    outputBuffer[0] = value.payload;
}

[numthreads(1, 1, 1)]
void partialParameterMain()
//PARTIAL_PARAMETER: entrypoint 'partialParameterMain' uses features that are not available
{
    outputBuffer[0] = useTrailing(1);
}

[numthreads(1, 1, 1)]
void partialGenericMain()
//PARTIAL_GENERIC: entrypoint 'partialGenericMain' uses features that are not available
{
    outputBuffer[0] = readTrailing<1>();
}

[numthreads(1, 1, 1)]
void foldedParameterMain()
//FOLDED_PARAMETER: entrypoint 'foldedParameterMain' uses features that are not available
{
    outputBuffer[0] = useFolded();
}

[numthreads(1, 1, 1)]
void foldedGenericMain()
//FOLDED_GENERIC: entrypoint 'foldedGenericMain' uses features that are not available
{
    outputBuffer[0] = readFolded();
}

[numthreads(1, 1, 1)]
void forwardedDefaultMain()
//FORWARDED_DEFAULT: entrypoint 'forwardedDefaultMain' uses features that are not available
{
    outputBuffer[0] = forward();
}

[numthreads(1, 1, 1)]
void bodyOverrideMain()
//BODY_OVERRIDE: entrypoint 'bodyOverrideMain' uses features that are not available
{
    // Overriding the default does not remove the requirement from the function body.
    outputBuffer[0] = useBody(Mode(1));
}

// HLSL: void hlslMain
[numthreads(1, 1, 1)]
void hlslMain()
{
    outputBuffer[0] = use();
    outputBuffer[1] = use(Mode(1));
    outputBuffer[2] = readDefault();
    outputBuffer[3] = readDefault<Mode(1)>();
    outputBuffer[4] = readTypeDefault();
    Holder<> value = { 7 };
    outputBuffer[5] = value.payload;
    outputBuffer[6] = useTrailing(1) + readTrailing<1>();
    outputBuffer[7] = useFolded() + readFolded();
    outputBuffer[8] = forward();
    outputBuffer[9] = useBody(Mode(1));
}
