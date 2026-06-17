//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// A type that conforms to the derived interface but whose inherited associated
// type does not satisfy the interface-level constraint must be rejected. The
// diagnostic is reported at the conforming type's inheritance clause
// (`inheritanceDecl->loc`), which the caret annotation below pins.

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
/*diag:
             ^^^^^^^^ error
             ^^^^^^^^ type argument 'Bad' does not conform to the required interface 'IDerived'
*/
{
    typealias DataType = int;
    int getData() { return 0; }
    int value() { return 1; }
}
