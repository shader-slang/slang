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
