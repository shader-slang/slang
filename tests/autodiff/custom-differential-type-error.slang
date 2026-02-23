//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv
// Using non-exhaustive because this triggers many cascading errors (E30027 at line 0) from IDifferentiable synthesis.

interface IFoo : IDifferentiable
{
    float myFunc();
}

struct Bar: IDifferentiable
{
    float y;
}

struct Foo :  IFoo
{
    typealias Differential = Bar;

    [DerivativeMember(Differential.x)]
/*CHECK:
                                   ^ 'x' is not a member of
     ^^^^^^^^^^^^^^^^ expected an expression of type 'float', got 'error'
     ^^^^^^^^^^^^^^^^ [DerivativeMember] must reference to a member in the associated differential type 'Bar'.
*/
    float x;

    [BackwardDifferentiable]
    float myFunc()
    {
        return x * x;
    };
}
