//TEST:COMPARE_COMPUTE(filecheck-buffer=VALUE): -cpu -xslang -std -xslang legacy -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=VALUE): -cpu -xslang -std -xslang 2025 -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=VALUE): -cpu -xslang -std -xslang 2026 -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=VALUE): -cpu -xslang -std -xslang 202c -output-using-type
//TEST:SIMPLE: -std legacy -no-codegen
//TEST:SIMPLE: -std 2025 -no-codegen
//TEST:SIMPLE: -std 2026 -no-codegen
//TEST:SIMPLE: -std 202c -no-codegen

interface IFoo
{
    int get();
}
interface IBar
{
}
typealias InterfaceAlias = IFoo;
typealias Both = InterfaceAlias & IBar;
typealias Box = dyn InterfaceAlias;

struct Foo : Both
{
    int get()
    {
        return 3;
    }
}

int constrained<T : Both>(T value)
{
    return value.get();
}
int boxed(Box value)
{
    return value.get();
}
int test(Foo value)
{
    return constrained(value) + boxed(value);
}

// Value tests ask about the payload, including through an interface alias or refinement.
interface IDerived : IFoo
{
}
bool sameInterface(dyn IFoo value)
{
    return value is IFoo;
}
bool aliasedInterface(dyn IFoo value)
{
    return value is InterfaceAlias;
}
bool baseInterface(dyn IDerived value)
{
    return value is IFoo;
}

// Default completion applies before choosing whether an interface should be boxed.
interface IValue<T = int>
{
}
struct Value : IValue<int>
{
}
typealias DefaultInterface = IValue;
dyn DefaultInterface defaultBox(Value value)
{
    return value;
}

// Generic interface aliases remain usable when the box's type arguments are substituted.
interface IGenericValue<T>
{
    T getValue();
}
typealias Contract<T> = IGenericValue<T>;
typealias GenericBox<T> = dyn Contract<T>;
struct GenericValue<T> : Contract<T>
{
    T value;
    T getValue()
    {
        return value;
    }
}
T readGenericBox<T>(GenericBox<T> value)
{
    return value.getValue();
}

struct DerivedFoo : IDerived
{
    int get()
    {
        return 3;
    }
}

// We observe both interface tests and the generic alias's payload after substitution.
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Foo value;
    DerivedFoo derivedValue;
    GenericValue<int> genericValue;
    genericValue.value = 11;
    outputBuffer[0] = sameInterface(value) ? 1u : 0u;
    outputBuffer[1] = aliasedInterface(value) ? 1u : 0u;
    outputBuffer[2] = baseInterface(derivedValue) ? 1u : 0u;
    outputBuffer[3] = uint(readGenericBox<int>(genericValue));
}

// VALUE: 1
// VALUE: 1
// VALUE: 1
// VALUE: 11
