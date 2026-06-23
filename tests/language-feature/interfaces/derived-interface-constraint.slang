//TEST:INTERPRET(filecheck=CHECK):

// Test interface-level `__constraint` declarations on a derived interface.
//
// A derived interface refines an associated type inherited from a base
// interface, constraining it to be equal to `This`. Generic code constrained
// by the derived interface can then treat `T.DataType` and `T` as equal.

interface IBase
{
    associatedtype DataType;
    DataType getData();
}

interface IDerived : IBase
{
    // `DataType` is inherited from IBase; constrain it to be `This`.
    __constraint DataType == This;
    int value();
}

int useEquality<T : IDerived>(T x)
{
    // `x.getData()` has type `T.DataType`. Because of the interface-level
    // constraint `This.DataType == This`, the compiler knows `T.DataType == T`,
    // so calling `value()` (a requirement of `IDerived`) on the result of
    // `getData()` type-checks.
    return x.getData().value();
}

struct Widget : IDerived
{
    typealias DataType = Widget;
    int payload;
    Widget getData() { return this; }
    int value() { return payload; }
}

void main()
{
    Widget w;
    w.payload = 42;

    // CHECK: 42
    printf("%d\n", useEquality(w));
}
