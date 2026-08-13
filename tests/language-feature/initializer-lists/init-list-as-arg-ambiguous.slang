// Regression test for https://github.com/shader-slang/slang/issues/11730.
//
// The fix for #11730 makes an initializer-list argument report itself as a
// viable coercion whenever the corresponding constructor actually type-checks
// (the `canCoerce` viability probe now agrees with real coercion). A concern
// raised on the issue is that, unlike a variable-declaration initializer or a
// return value, a function-call argument must go through overload resolution,
// so making more constructors viable must not cause the compiler to silently
// pick one overload when several are equally applicable.
//
// Consider the call `overloadedFunc({a, b})` below, where `a` is an `int2` and
// `b` is an `int`. The brace initializer can form both an `int3` (flattening
// the `int2` into two components) and an `ABC` (whose fields are exactly
// `int2` then `int`), so two overloads are viable and the call must be
// diagnosed as ambiguous rather than silently resolved. Before the fix this
// call instead produced a spurious `error 30019: type mismatch` because the
// `int3` candidate was wrongly rejected during overload resolution.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct ABC
{
    int2 a;
    int b;
}

int overloadedFunc(int2 a, int b)
{
    return a.x + a.y + b + 1000;
}

int overloadedFunc(int3 v)
{
    return v.x + v.y + v.z + 1001;
}

int overloadedFunc(ABC v)
{
    return v.a.x + v.a.y + v.b + 1002;
}

RWStructuredBuffer<int> output;

void test()
{
    int2 a = {1, 2};
    int b = 3;
    output[0] = overloadedFunc({a, b});
    //CHECK: ambiguous call to 'overloadedFunc'
}
