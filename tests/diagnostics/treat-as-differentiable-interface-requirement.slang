//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// Test that [TreatAsDifferentiable] cannot be applied to interface requirements.

interface IFoo
{
    // CHECK: ([[# @LINE+2]]): error 30313
    [TreatAsDifferentiable]
    float compute(float x);
}

// OK: [TreatAsDifferentiable] on a concrete function is fine.
[TreatAsDifferentiable]
float bar(float x)
{
    return x * x;
}

interface IBar
{
    // CHECK: ([[# @LINE+2]]): error 30313
    [TreatAsDifferentiable]
    float genericReq<T : IFloat>(T x);
}

// OK: [TreatAsDifferentiable] on a concrete generic function is fine.
[TreatAsDifferentiable]
float genericBar<T : IFloat>(T x)
{
    return float(x);
}

struct Impl : IFoo
{
    // OK: [TreatAsDifferentiable] on a concrete method implementing an interface is fine.
    [TreatAsDifferentiable]
    float compute(float x)
    {
        return x;
    }
}

struct Impl2
{
    // OK: [TreatAsDifferentiable] on a concrete generic method is fine.
    [TreatAsDifferentiable]
    float genericMethod<T : IFloat>(T x)
    {
        return float(x);
    }
}
