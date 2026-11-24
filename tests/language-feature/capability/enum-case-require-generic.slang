//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry hlslMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry overloadMain -stage compute -emit-spirv-directly
//DIAGNOSTIC_TEST:SIMPLE(diag=NAMED,non-exhaustive):-target spirv -entry namedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CANCELLED,non-exhaustive):-target spirv -entry cancelledMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=ALIAS,non-exhaustive):-target spirv -entry aliasMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=SIZE,non-exhaustive):-target spirv -entry sizeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=MEMBER,non-exhaustive):-target spirv -entry memberMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PARTIAL,non-exhaustive):-target spirv -entry partialMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=OVERLOADED,non-exhaustive):-target spirv -entry overloadedMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PARAMETER,non-exhaustive):-target spirv -entry parameterMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=RESULT,non-exhaustive):-target spirv -entry resultMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=GLOBAL,non-exhaustive):-target spirv -entry globalMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC_MEMBER,non-exhaustive):-target spirv -entry genericMemberMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PARENT_MEMBER,non-exhaustive):-target spirv -entry parentMemberMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FORWARD_ALIAS,non-exhaustive):-target spirv -entry forwardAliasMain -stage compute
// non-exhaustive: capability errors also include declaration/use notes.

enum Mode
{
    [require(hlsl)] A = 0,
    [require(spirv)] B = 1,
}

// The source argument requires A even though the generic body never reads V.
int readValue<let V : Mode>() { return 7; }
int readInt<let V : int>() { return V; }
int readPartial<let V : Mode, T>(T value) { return 7; }

int readOverloaded<let V : Mode>(int value) { return value; }
[require(hlsl)]
int readOverloaded<let V : Mode>(float value) { return int(value); }

struct Holder<let V : Mode>
{
    int payload;
    static int read() { return 9; }
    static int readGeneric<T>() { return 11; }
}

struct Container
{
    static int read<let V : Mode>() { return 13; }
}

// The body need not mention A for its signature to contribute A's requirement.
int readHolder(Holder<Mode.A> value) { return value.payload; }
Holder<Mode.A> makeHolder() { return { 7 }; }

// The reference contributes a requirement even though the stored constant is zero.
static const int hlslConstant = int(Mode.A) * 0;

struct IntegerHolder<let V : int>
{
    int payload;
}

// The alias infers its requirement before folding discards the reference to A.
typealias HlslAlias = IntegerHolder<int(Mode.A) * 0>;

int readForwardAlias()
{
    // The alias is declared later; its source reference to A must be checked before use.
    Selected<1> value = { 7 };
    return value.payload;
}

[require(hlsl)]
int selectValue<T>(T value) { return 0; }
int selectValue(int value) { return value; }

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void namedMain()
//NAMED: entrypoint 'namedMain' uses features that are not available
{
    outputBuffer[0] = readValue<Mode.A>();
}

[numthreads(1, 1, 1)]
void cancelledMain()
//CANCELLED: entrypoint 'cancelledMain' uses features that are not available
{
    outputBuffer[0] = readInt<int(Mode.A) * 0>();
}

[numthreads(1, 1, 1)]
void aliasMain()
//ALIAS: entrypoint 'aliasMain' uses features that are not available
{
    outputBuffer[0] = readInt<sizeof(HlslAlias)>();
}

[numthreads(1, 1, 1)]
void sizeMain()
//SIZE: entrypoint 'sizeMain' uses features that are not available
{
    outputBuffer[0] = readInt<sizeof(Holder<Mode.A>)>();
}

[numthreads(1, 1, 1)]
void memberMain()
//MEMBER: entrypoint 'memberMain' uses features that are not available
{
    outputBuffer[0] = Holder<Mode.A>.read();
}

[numthreads(1, 1, 1)]
void partialMain()
//PARTIAL: entrypoint 'partialMain' uses features that are not available
{
    outputBuffer[0] = readPartial<Mode.A>(0);
}

[numthreads(1, 1, 1)]
void overloadedMain()
//OVERLOADED: entrypoint 'overloadedMain' uses features that are not available
{
    outputBuffer[0] = readOverloaded<Mode.A>(1);
}

[numthreads(1, 1, 1)]
void parameterMain()
//PARAMETER: entrypoint 'parameterMain' uses features that are not available
{
    outputBuffer[0] = readHolder({ 7 });
}

[numthreads(1, 1, 1)]
void resultMain()
//RESULT: entrypoint 'resultMain' uses features that are not available
{
    outputBuffer[0] = makeHolder().payload;
}

[numthreads(1, 1, 1)]
void globalMain()
//GLOBAL: entrypoint 'globalMain' uses features that are not available
{
    outputBuffer[0] = hlslConstant;
}

[numthreads(1, 1, 1)]
void genericMemberMain()
//GENERIC_MEMBER: entrypoint 'genericMemberMain' uses features that are not available
{
    outputBuffer[0] = Container.read<Mode.A>();
}

[numthreads(1, 1, 1)]
void parentMemberMain()
//PARENT_MEMBER: entrypoint 'parentMemberMain' uses features that are not available
{
    outputBuffer[0] = Holder<Mode.A>.readGeneric<int>();
}

[numthreads(1, 1, 1)]
void forwardAliasMain()
//FORWARD_ALIAS: entrypoint 'forwardAliasMain' uses features that are not available
{
    outputBuffer[0] = readForwardAlias();
}

// HLSL: void hlslMain
[numthreads(1, 1, 1)]
void hlslMain()
{
    outputBuffer[0] = readValue<Mode.A>();
    outputBuffer[1] = readInt<int(Mode.A) * 0>();
    outputBuffer[2] = readInt<sizeof(HlslAlias)>();
    outputBuffer[3] = readInt<sizeof(Holder<Mode.A>)>();
    outputBuffer[4] = Holder<Mode.A>.read();
    outputBuffer[5] = readPartial<Mode.A>(0);
    outputBuffer[6] = readOverloaded<Mode.A>(1);
    outputBuffer[7] = readHolder({ 7 });
    outputBuffer[8] = makeHolder().payload;
    outputBuffer[9] = hlslConstant;
    outputBuffer[10] = Container.read<Mode.A>();
    outputBuffer[11] = Holder<Mode.A>.readGeneric<int>();
    outputBuffer[12] = readForwardAlias();
}

// SPIRV: OpEntryPoint GLCompute
[numthreads(1, 1, 1)]
void overloadMain()
{
    // Only the selected overload contributes requirements.
    outputBuffer[0] = selectValue(1);
    outputBuffer[1] = readValue<Mode.B>();
    // Retain the source generic argument without visiting the unselected float overload.
    outputBuffer[2] = readOverloaded<Mode.B>(1);
}

// The generic parameter can disappear from the numeric value without removing A's requirement.
typealias Selected<let N : int> = IntegerHolder<int(Mode.A) * N * 0>;
