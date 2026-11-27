//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

struct InvalidSetters
{
    property int outValue
    {
        set(out int value) {}
//diag:             ^^^^^ a 'set' parameter must be input-only
//diag:             ^^^^^ 'set' parameter 'value' has parameter-passing mode 'out'; only 'in' and immutable-borrow modes are allowed
    }

    property int inOutValue
    {
        set(inout int value) {}
//diag:               ^^^^^ a 'set' parameter must be input-only
//diag:               ^^^^^ 'set' parameter 'value' has parameter-passing mode 'inout'; only 'in' and immutable-borrow modes are allowed
    }

    property int refValue
    {
        set(__ref int value) {}
//diag:               ^^^^^ a 'set' parameter must be input-only
//diag:               ^^^^^ 'set' parameter 'value' has parameter-passing mode 'ref'; only 'in' and immutable-borrow modes are allowed
    }

    property int defaultedValue
    {
        set(int value = 0) {}
//diag:                 ^ a 'set' parameter cannot have a default value
//diag:                 ^ the value passed to a 'set' accessor is always supplied by an assignment
    }
}

interface IIndex {}
struct ConcreteIndex : IIndex {}

struct InvalidSubscripts
{
    __subscript(out IIndex index) -> int
//diag:                    ^^^^^ a subscript parameter must be input-only
//diag:                    ^^^^^ subscript parameter 'index' has parameter-passing mode 'out'; only 'in' and immutable-borrow modes are allowed
    {
        get { return 0; }
    }

    __subscript(inout int index) -> int
//diag:                   ^^^^^ a subscript parameter must be input-only
//diag:                   ^^^^^ subscript parameter 'index' has parameter-passing mode 'inout'; only 'in' and immutable-borrow modes are allowed
    {
        get { return 0; }
    }

    __subscript(__ref int index) -> int
//diag:                   ^^^^^ a subscript parameter must be input-only
//diag:                   ^^^^^ subscript parameter 'index' has parameter-passing mode 'ref'; only 'in' and immutable-borrow modes are allowed
    {
        get { return 0; }
    }
}

int exerciseInvalidSubscriptRecovery()
{
    InvalidSubscripts storage;
    ConcreteIndex index;
    return storage[index];
//diag:           ^ concrete type passed to interface-typed output parameter
//diag:           ^ argument passed to parameter 'index' is of concrete type 'ConcreteIndex', but interface-typed output parameters require interface-typed arguments
}
