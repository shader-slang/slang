//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that [TreatAsDifferentiable] cannot be applied to interface requirements.

interface IFoo
{
    [TreatAsDifferentiable]
/*CHECK:
     ^^^^^^^^^^^^^^^^^^^^^ [TreatAsDifferentiable] cannot be applied to interface requirement
     ^^^^^^^^^^^^^^^^^^^^^ '[TreatAsDifferentiable]' cannot be applied to an interface requirement; it can only be applied to concrete function definitions.
*/
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
    [TreatAsDifferentiable]
/*CHECK:
     ^^^^^^^^^^^^^^^^^^^^^ [TreatAsDifferentiable] cannot be applied to interface requirement
     ^^^^^^^^^^^^^^^^^^^^^ '[TreatAsDifferentiable]' cannot be applied to an interface requirement; it can only be applied to concrete function definitions.
*/
    float genericReq<T : IFloat>(T x);
}

// OK: [TreatAsDifferentiable] on a concrete generic function is fine.
[TreatAsDifferentiable]
float genericBar<T : IFloat>(T x)
{
    return x.toFloat();
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
        return x.toFloat();
    }
}
