// Companion to `fwd-diff-generic-constrained-member.slang` for
// shader-slang/slang#11004. Pins the existing front-end behaviour for
// `__fwd_diff(method-with-default-arg)` when the user omits the
// default-valued trailing argument.
//
// Reviewer-flagged risk surface (round 6): if the user-supplied
// argument list is shorter than the method's full parameter list and
// reaches the AD-2.0 lowering, the new `funcTypeAlreadyHasImplicitThis`
// gate `paramCount == argCount + 1` is false (because
// `FwdDiffFuncType::_resolveImplOverride` enumerates ALL base
// parameters, not just the user-supplied ones), execution falls
// through to the else-branch, and `addDirectCallArgs(expr,
// resolvedFuncType, ...)` asserts on `argCount ==
// funcType->getParamCount()` — the same `paramCount == callArgCount`
// failure mode the parent PR sets out to fix, just shifted by one.
//
// In practice the front end's argument-count check (`E39999`) fires
// before that path is reachable: Slang's `__fwd_diff(method)` does not
// expand defaults at the AST level, so the user must supply every
// argument, and an under-application is rejected before lowering. This
// test pins that diagnostic so a future change that weakens the
// argument-count check (and would expose the AD-2.0 lowering to the
// short-application case) fails this test rather than silently
// re-enabling the assertion.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

interface IFoo : IDifferentiable
{
    [Differentiable]
    float bar(float x, float y);
}

struct Foo : IFoo, IDifferentiable
{
    [Differentiable]
    [NoDiffThis]
    float bar(float x, float y = 0.0) { return x + y; }
}

[Differentiable]
float test<F : IFoo>(F foo, float t)
{
    // CHECK: error 39999: not enough arguments to call (got 1, expected 2)
    return __fwd_diff(foo.bar)(diffPair(t, 1.0)).d;
}
