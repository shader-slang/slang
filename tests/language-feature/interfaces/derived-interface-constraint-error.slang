//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// A type that conforms to the derived interface but whose inherited associated
// type does not satisfy the interface-level constraint must be rejected.

interface IBase
{
    associatedtype DataType;
    DataType getData();
}

interface IDerived : IBase
{
    __constraint DataType == This;
    int value();
}

// `Bad.DataType` is `int`, which is not `Bad`, so the `DataType == This`
// constraint of `IDerived` is not satisfied.
struct Bad : IDerived
{
    typealias DataType = int;
    int getData() { return 0; }
    int value() { return 1; }
}
//CHECK: error[E38029]: type argument doesn't conform to interface
