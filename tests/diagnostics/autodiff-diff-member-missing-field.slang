//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that error 30102 is emitted when a differentiable member does not
// have a corresponding field in the custom Differential type.

struct MyDiff : IDifferentiable
{
    typealias Differential = MyDiff;
    float dy;

    static MyDiff dzero() { return { 0.0 }; }
    static MyDiff dadd(MyDiff a, MyDiff b) { return { a.dy + b.dy }; }
    static MyDiff dmul(float a, MyDiff b) { return { a * b.dy }; }
}

struct MyType : IDifferentiable
{
    typealias Differential = MyDiff;
    float x;
//CHECK:  ^ missing field in differential type
//CHECK:  ^ differentiable member 'x' should have a corresponding field in 'MyDiff'. Use [DerivativeMember(...)] or mark as no_diff
    float y;
//CHECK:  ^ missing field in differential type
//CHECK:  ^ differentiable member 'y' should have a corresponding field in 'MyDiff'. Use [DerivativeMember(...)] or mark as no_diff

    static MyDiff dzero() { return MyDiff.dzero(); }
    static MyDiff dadd(MyDiff a, MyDiff b) { return MyDiff.dadd(a, b); }
    static MyDiff dmul(float a, MyDiff b) { return MyDiff.dmul(a, b); }
}
