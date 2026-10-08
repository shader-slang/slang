//TEST:INTERPRET(filecheck=CHECK):

// Test the *subtype* form of an interface-level `__constraint` (`X : IFoo`),
// as opposed to the equality form (`X == This`) exercised by
// `derived-interface-constraint.slang`.
//
// A derived interface refines an associated type inherited from a base
// interface, constraining it to conform to another interface. Generic code
// constrained by the derived interface can then call that interface's methods
// on values of the associated type.

interface ICounter
{
    int count();
}

interface IBase
{
    associatedtype DataType;
    DataType getData();
}

interface IDerived : IBase
{
    // `DataType` is inherited from IBase; constrain it to conform to ICounter.
    __constraint DataType : ICounter;
}

int useSubtype<T : IDerived>(T x)
{
    // `x.getData()` has type `T.DataType`. Because of the interface-level
    // constraint `DataType : ICounter`, the compiler knows `T.DataType` conforms
    // to `ICounter`, so calling `count()` on the result type-checks.
    return x.getData().count();
}

struct Item : ICounter
{
    int n;
    int count() { return n; }
}

struct Holder : IDerived
{
    typealias DataType = Item;
    Item item;
    Item getData() { return item; }
}

void main()
{
    Holder h;
    h.item.n = 7;

    // CHECK: 7
    printf("%d\n", useSubtype(h));
}
