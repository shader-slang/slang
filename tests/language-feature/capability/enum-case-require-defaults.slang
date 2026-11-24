//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry hlslMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PARAMETER_DEFAULT,non-exhaustive):-target spirv -entry parameterDefaultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PARAMETER_OVERRIDE,non-exhaustive):-target spirv -entry parameterOverrideMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC_DEFAULT,non-exhaustive):-target spirv -entry genericDefaultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC_OVERRIDE,non-exhaustive):-target spirv -entry genericOverrideMain -stage compute
// non-exhaustive: capability errors also include declaration/use notes.

enum Mode
{
    [require(hlsl)] A = 0,
}

// Each declaration references A in its default, regardless of which arguments a caller supplies.
int use(Mode mode = Mode.A) { return int(mode); }
int readDefault<let V : Mode = Mode.A>() { return 7; }

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void parameterDefaultMain()
//PARAMETER_DEFAULT: entrypoint 'parameterDefaultMain' uses features that are not available
{
    outputBuffer[0] = use();
}

[numthreads(1, 1, 1)]
void parameterOverrideMain()
//PARAMETER_OVERRIDE: entrypoint 'parameterOverrideMain' uses features that are not available
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
//GENERIC_OVERRIDE: entrypoint 'genericOverrideMain' uses features that are not available
{
    outputBuffer[0] = readDefault<Mode(1)>();
}

// HLSL: void hlslMain
[numthreads(1, 1, 1)]
void hlslMain()
{
    outputBuffer[0] = use();
    outputBuffer[1] = use(Mode(1));
    outputBuffer[2] = readDefault();
    outputBuffer[3] = readDefault<Mode(1)>();
}
