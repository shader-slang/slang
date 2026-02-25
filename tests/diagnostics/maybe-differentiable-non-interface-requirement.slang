//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// Test that [MaybeDifferentiable] can only be applied to interface requirements.

interface IFoo
{
    // OK: [MaybeDifferentiable] on an interface requirement is fine.
    [MaybeDifferentiable]
    float compute(float x);
}

// CHECK: ([[# @LINE+2]]): error 30314
[MaybeDifferentiable]
float bar(float x)
{
    return x * x;
}

struct MyType
{
    // CHECK: ([[# @LINE+2]]): error 30314
    [MaybeDifferentiable]
    float compute(float x)
    {
        return x;
    }
}

interface IBar
{
    // OK: [MaybeDifferentiable] on a generic interface requirement is fine.
    [MaybeDifferentiable]
    float genericReq<T : IFloat>(T x);
}

// CHECK: ([[# @LINE+2]]): error 30314
[MaybeDifferentiable]
T genericBar<T : IFloat>(T x)
{
    return T(x);
}

struct MyType2
{
    // CHECK: ([[# @LINE+2]]): error 30314
    [MaybeDifferentiable]
    T genericMethod<T : IFloat>(T x)
    {
        return T(x);
    }
}
