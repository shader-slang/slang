//TEST:SIMPLE(filecheck=HLSL):-std legacy -target hlsl -entry hlslMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=IDENTITY,non-exhaustive):-std legacy -target spirv -entry identityMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=INITIALIZER,non-exhaustive):-std legacy -target spirv -entry initializerMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=ALIAS,non-exhaustive):-std legacy -target spirv -entry aliasMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=LEGACY,non-exhaustive):-std legacy -target spirv -entry legacyMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=ENUM_IDENTITY,non-exhaustive):-std legacy -target spirv -entry enumIdentityMain -stage compute
// non-exhaustive: capability errors also include declaration/use notes.
// Legacy mode includes the C-style aggregate initialization case below.

enum Mode
{
    [require(hlsl)] A = 0,
}

struct Holder<let V : Mode>
{
    int payload;
}

typealias HlslAlias = Holder<Mode.A>;

int readValue<let V : Mode>() { return int(V); }

int identity()
{
    Holder<Mode(0)> value = { 7 };
    // The canonical types are equal; the explicitly written target still references A.
    return Holder<Mode.A>(value).payload;
}

int initializer()
{
    // Aggregate initialization also retains the explicitly written target's references.
    return Holder<Mode.A>(7).payload;
}

int aliasConstructor()
{
    return HlslAlias(7).payload;
}

int legacyCast()
{
    Holder<Mode(0)> value = (Holder<Mode.A>)0;
    return value.payload;
}

int enumIdentity()
{
    // Keeping the source reference must not stop an identity cast from being a constant argument.
    return readValue<Mode(Mode.A)>();
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void identityMain()
//IDENTITY: entrypoint 'identityMain' uses features that are not available
{
    outputBuffer[0] = identity();
}

[numthreads(1, 1, 1)]
void initializerMain()
//INITIALIZER: entrypoint 'initializerMain' uses features that are not available
{
    outputBuffer[0] = initializer();
}

[numthreads(1, 1, 1)]
void aliasMain()
//ALIAS: entrypoint 'aliasMain' uses features that are not available
{
    outputBuffer[0] = aliasConstructor();
}

[numthreads(1, 1, 1)]
void legacyMain()
//LEGACY: entrypoint 'legacyMain' uses features that are not available
{
    outputBuffer[0] = legacyCast();
}

[numthreads(1, 1, 1)]
void enumIdentityMain()
//ENUM_IDENTITY: entrypoint 'enumIdentityMain' uses features that are not available
{
    outputBuffer[0] = enumIdentity();
}

// HLSL: void hlslMain
[numthreads(1, 1, 1)]
void hlslMain()
{
    outputBuffer[0] = identity();
    outputBuffer[1] = initializer();
    outputBuffer[2] = aliasConstructor();
    outputBuffer[3] = legacyCast();
    outputBuffer[4] = enumIdentity();
}
