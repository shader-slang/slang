// Companion to `fwd-diff-generic-constrained-member.slang`. Pins the
// existing front-end behaviour for
// `__fwd_diff(method-with-default-arg)` when the user omits the
// default-valued trailing argument.
//
// `__fwd_diff(method)` does not expand default arguments, so the user must
// supply every argument; an under-application is rejected by the front
// end's argument-count check before any receiver is threaded through.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

interface IFoo : IDifferentiable
{
    [Differentiable]
    float bar(float x, float y);
//CHECK:  ^^^ candidate: func F.bar(float, float) -> float
}

struct Foo : IFoo, IDifferentiable
{
    [Differentiable]
    [NoDiffThis]
    float bar(float x, float y = 0.0) { return x + y; }
//CHECK:  ^^^ candidate: func Foo.bar(float, float) -> float
}

[Differentiable]
float test<F : IFoo>(F foo, float t)
{
    return __fwd_diff(foo.bar)(diffPair(t, 1.0)).d;
//CHECK:                      ^ not enough arguments to call
//CHECK:                      ^ not enough arguments to call (got 1, expected 2)
}

// Through a concrete receiver the call does reach `Foo.bar`, whose `y` has a default.
[Differentiable]
float testConcrete(Foo foo, float t)
{
    return __fwd_diff(foo.bar)(diffPair(t, 1.0)).d;
//CHECK:                      ^ not enough arguments to call
//CHECK:                      ^ not enough arguments to call (got 1, expected 2)
}
