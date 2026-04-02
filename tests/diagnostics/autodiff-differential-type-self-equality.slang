//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that error 30096 is emitted when a differential type's own
// Differential is not itself.

struct BadDiff : IDifferentiable
{
    typealias Differential = float;

    static Differential dzero() { return 0.0; }
    static Differential dadd(Differential a, Differential b) { return a + b; }
    static Differential dmul(float a, Differential b) { return a * b; }
}

struct MyType : IDifferentiable
//CHECK:        ^^^^^^^^^^^^^^^ invalid differential type
//CHECK:        ^^^^^^^^^^^^^^^ cannot use type 'BadDiff' as a `Differential` type. A differential type's differential must be itself. However, the Differential of 'BadDiff' is 'float'.
{
    float val;

    typealias Differential = BadDiff;

    static BadDiff dzero() { return {}; }
    static BadDiff dadd(BadDiff a, BadDiff b) { return a; }
    static BadDiff dmul(float a, BadDiff b) { return b; }
}
