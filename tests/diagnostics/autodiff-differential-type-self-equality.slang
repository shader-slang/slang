//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that error 30096 is emitted when a differential type's own
// Differential is not itself.

struct BadDiff : IDifferentiable
{
    typealias Differential = float;
}

struct MyType : IDifferentiable
//CHECK:        ^^^^^^^^^^^^^^^ invalid differential type
//CHECK:        ^^^^^^^^^^^^^^^ cannot use type 'BadDiff' as a `Differential` type. A differential type's differential must be itself. However, the Differential of 'BadDiff' is 'float'.
{
    float val;

    typealias Differential = BadDiff;
}
