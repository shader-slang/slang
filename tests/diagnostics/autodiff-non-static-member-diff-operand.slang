//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A differentiation operator accepts either a free function, a static method, or an unbound
// instance method such as `Foo::compute`. The expression `foo.compute` instead binds `foo` as the
// receiver. Differentiation IR has no representation for that captured receiver, so semantic
// checking rejects the bound reference before lowering.

struct Foo
{
    [Differentiable]
    float compute(float t)
    {
        return t;
    }
}

void test()
{
    Foo foo;
    __fwd_diff(foo.compute)(DifferentialPair<float>(1.0, 1.0));
//CHECK:           ^^^^^^^ non-static function reference not allowed
//CHECK:           ^^^^^^^ non-static function reference 'compute' is not allowed here.
}

struct OverloadedFoo
{
    [ForwardDifferentiable]
    static float compute(float value)
    {
        return value;
    }

    [ForwardDifferentiable]
    float compute(float2 value)
    {
        return value.x + value.y;
    }
}

void testOverloadSelection()
{
    OverloadedFoo foo;

    // Both candidates remain available during overload resolution. The scalar argument makes
    // overload resolution select the static method, which is valid even though it was written
    // through a value.
    __fwd_diff(foo.compute)(DifferentialPair<float>(1.0, 1.0));

    // The vector argument makes overload resolution select the instance method. Final
    // differentiation-operand validation then rejects it because the expression binds `foo` as
    // its receiver.
    __fwd_diff(foo.compute)(DifferentialPair<float2>(float2(1.0), float2(1.0)));
//CHECK:           ^^^^^^^ non-static function reference not allowed
//CHECK:           ^^^^^^^ non-static function reference 'compute' is not allowed here.
}
