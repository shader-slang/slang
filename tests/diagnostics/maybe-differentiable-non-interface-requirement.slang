//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that [MaybeDifferentiable] can only be applied to interface requirements.

interface IFoo
{
    // OK: [MaybeDifferentiable] on an interface requirement is fine.
    [MaybeDifferentiable]
    float compute(float x);
}

[MaybeDifferentiable]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^^ [MaybeDifferentiable] cannot be applied to non-interface requirement
 ^^^^^^^^^^^^^^^^^^^ '[MaybeDifferentiable]' cannot be applied to non-interface requirement; it can only be applied to interface requirements.
*/
float bar(float x)
{
    return x * x;
}

struct MyType
{
[MaybeDifferentiable]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^^ [MaybeDifferentiable] cannot be applied to non-interface requirement
 ^^^^^^^^^^^^^^^^^^^ '[MaybeDifferentiable]' cannot be applied to non-interface requirement; it can only be applied to interface requirements.
*/
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

[MaybeDifferentiable]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^^ [MaybeDifferentiable] cannot be applied to non-interface requirement
 ^^^^^^^^^^^^^^^^^^^ '[MaybeDifferentiable]' cannot be applied to non-interface requirement; it can only be applied to interface requirements.
*/
T genericBar<T : IFloat>(T x)
{
    return T(x);
}

struct MyType2
{
[MaybeDifferentiable]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^^ [MaybeDifferentiable] cannot be applied to non-interface requirement
 ^^^^^^^^^^^^^^^^^^^ '[MaybeDifferentiable]' cannot be applied to non-interface requirement; it can only be applied to interface requirements.
*/
    T genericMethod<T : IFloat>(T x)
    {
        return T(x);
    }
}
