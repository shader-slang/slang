//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Differentiating a method bound to an instance, as `fwd_diff(obj.method)`, is
// rejected: a [Differentiable] function that takes the instance and calls the
// method is differentiated instead.

interface ICurve
{
    [Differentiable]
    float eval(float t);
}

struct Foo
{
    float x;

    [Differentiable]
    [NoDiffThis]
    float compute(float t) { return x * t; }

    [Differentiable]
    [NoDiffThis]
    float compute(float2 t) { return x * t.x; }

    [Differentiable]
    static float scale(float t) { return 2.0 * t; }

    // A derivative attribute names a method of the same type.
    [ForwardDerivativeOf(scale)]
    static DifferentialPair<float> scaleFwd(DifferentialPair<float> t)
    {
        return diffPair(2.0 * t.p, 2.0 * t.d);
    }

    [Differentiable]
    [NoDiffThis]
    float useImplicitThis(float t)
    {
        return __fwd_diff(compute)(diffPair(t, 1.0)).d;
//CHECK:                  ^^^^^^^ non-static function reference not allowed
//CHECK:                  ^^^^^^^ non-static function reference 'compute' is not allowed here; differentiate a [Differentiable] function that takes the instance as a parameter and calls it instead.
    }

    [Differentiable]
    [NoDiffThis]
    float useThis(float t)
    {
        return __fwd_diff(this.compute)(diffPair(t, 1.0)).d;
//CHECK:                       ^^^^^^^ non-static function reference not allowed
//CHECK:                       ^^^^^^^ non-static function reference 'compute' is not allowed here; differentiate a [Differentiable] function that takes the instance as a parameter and calls it instead.
    }
}

// A derivative attribute names a method, so an explicit `this` is accepted.
struct Scaled
{
    float scale;

    [NoDiffThis]
    float eval(float x) { return scale * x * x; }

    [NoDiffThis]
    [ForwardDerivativeOf(this.eval)]
    DifferentialPair<float> evalForward(DifferentialPair<float> x)
    {
        return diffPair(scale * x.p * x.p, scale * 2.0 * x.p * x.d);
    }
}

// Through an instance, only the static overload can be differentiated.
struct Mixed
{
    [Differentiable]
    static float compute(float t) { return t * t; }

    [Differentiable]
    [NoDiffThis]
    float compute(float2 t) { return t.x; }
}

float testMixed(Mixed mixed)
{
    return __fwd_diff(mixed.compute)(diffPair(3.0, 1.0)).d;
}

void test()
{
    Foo foo;
    foo.x = 1.0;
    __fwd_diff(foo.compute)(DifferentialPair<float>(1.0, 1.0));
//CHECK:           ^^^^^^^ non-static function reference not allowed
//CHECK:           ^^^^^^^ non-static function reference 'compute' is not allowed here; differentiate a [Differentiable] function that takes the instance as a parameter and calls it instead.

    __fwd_diff(__fwd_diff(foo.compute))(diffPair(diffPair(1.0, 1.0), diffPair(1.0, 0.0)));
//CHECK:                      ^^^^^^^ non-static function reference not allowed
//CHECK:                      ^^^^^^^ non-static function reference 'compute' is not allowed here; differentiate a [Differentiable] function that takes the instance as a parameter and calls it instead.

    // A static method may still be referenced through an instance.
    __fwd_diff(foo.scale)(DifferentialPair<float>(1.0, 1.0));
}

float testGeneric<C : ICurve>(C c, float t)
{
    var p = diffPair(t);
    __bwd_diff(c.eval)(p, 1.0);
//CHECK:         ^^^^ non-static function reference not allowed
//CHECK:         ^^^^ non-static function reference 'eval' is not allowed here; differentiate a [Differentiable] function that takes the instance as a parameter and calls it instead.
    return __fwd_diff(c.eval)(diffPair(t, 1.0)).d;
//CHECK:                ^^^^ non-static function reference not allowed
//CHECK:                ^^^^ non-static function reference 'eval' is not allowed here; differentiate a [Differentiable] function that takes the instance as a parameter and calls it instead.
}

float testExistential(ICurve c, float t)
{
    return __fwd_diff(c.eval)(diffPair(t, 1.0)).d;
//CHECK:                ^^^^ non-static function reference not allowed
//CHECK:                ^^^^ non-static function reference 'eval' is not allowed here; differentiate a [Differentiable] function that takes the instance as a parameter and calls it instead.
}
