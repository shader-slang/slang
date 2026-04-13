//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that error 30310 is emitted when [DerivativeMember] is applied
// to a member whose type is not differentiable.

struct MyDiff : IDifferentiable
{
    typealias Differential = MyDiff;
    float dx;

    static MyDiff dzero() { return { 0.0 }; }
    static MyDiff dadd(MyDiff a, MyDiff b) { return { a.dx + b.dx }; }
    static MyDiff dmul(float a, MyDiff b) { return { a * b.dx }; }
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

    static MyDiff dzero() { return MyDiff.dzero(); }
    static MyDiff dadd(MyDiff a, MyDiff b) { return MyDiff.dadd(a, b); }
    static MyDiff dmul(float a, MyDiff b) { return MyDiff.dmul(a, b); }
}
