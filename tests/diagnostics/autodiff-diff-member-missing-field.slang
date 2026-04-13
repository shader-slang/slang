//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that error 30102 is emitted when a differentiable member does not
// have a corresponding field in the custom Differential type.

struct MyDiff : IDifferentiable
{
    typealias Differential = MyDiff;
    float dy;
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
}
