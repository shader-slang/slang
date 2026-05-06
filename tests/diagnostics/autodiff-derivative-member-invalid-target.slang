//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that [DerivativeMember] must reference a member in the expected differential type (error 31130).

struct Bar : IDifferentiable
{
    typealias Differential = Bar;
    float y;
}

struct Foo : IDifferentiable
{
    typealias Differential = Bar;

    [DerivativeMember(Differential.nonexistent)]
/*CHECK:
                                   ^^^^^^^^^^^ 'nonexistent' is not a member of
     ^^^^^^^^^^^^^^^^ [DerivativeMember] must reference to a member in the associated differential type 'Bar'.
*/
    float x;
}
