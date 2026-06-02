//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that error 30310 is emitted when [DerivativeMember] is applied
// to a member whose type is not differentiable.

struct MyDiff : IDifferentiable
{
    typealias Differential = MyDiff;
    float dx;

    [Differentiable]
    static MyDiff dzero() { return { 0.0 }; }

    [Differentiable]
    static MyDiff dadd(MyDiff a, MyDiff b) { return { a.dx + b.dx }; }
}

struct MyType : IDifferentiable
{
    typealias Differential = MyDiff;

    [DerivativeMember(MyDiff.dx)]
    float x;

    [DerivativeMember(MyDiff.dx)]
/*CHECK:
     ^^^^^^^^^^^^^^^^ type is not differentiable
     ^^^^^^^^^^^^^^^^ type 'int' is not differentiable.
*/
    int notDiff;

    [Differentiable]
    static MyDiff dzero() { return MyDiff.dzero(); }

    [Differentiable]
    static MyDiff dadd(MyDiff a, MyDiff b) { return MyDiff.dadd(a, b); }
}
