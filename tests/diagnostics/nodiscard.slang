//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for shader-slang/slang#11454.
// A `[nodiscard]`-marked function whose non-void result is discarded at a
// top-level statement must warn (E30059). Target-independent: this is a
// front-end check, so it needs no GPU/target.
//
// Coverage spans the shapes the feature is actually deployed against — a
// generic static method (the `CoopMat::Load` family) and the
// static-method-called-as-instance form — plus negative cases (a void result
// and a used result) which must NOT warn.

[nodiscard]
int computeValue()
{
    return 42;
}

[nodiscard]
T genericCompute<T>(T x)
{
    return x;
}

struct Loader
{
    // Generic static method — the same shape as `CoopMat::Load<...>`.
    [nodiscard]
    static T load<T>(T x)
    {
        return x;
    }
}

// `[nodiscard]` on a void-returning function: there is no result to discard,
// so a discarded call must NOT warn.
[nodiscard]
void doNothing()
{
}

// Using the result (returning it) must NOT warn.
int useResult()
{
    return computeValue();
}

void test()
{
    // Plain free function, discarded.
    computeValue();
//CHECK: ignoring return value of function declared with '[nodiscard]' attribute

    // Generic free function, discarded.
    genericCompute<int>(3);
//CHECK: ignoring return value of function declared with '[nodiscard]' attribute

    // Generic static method, discarded.
    Loader.load<int>(5);
//CHECK: ignoring return value of function declared with '[nodiscard]' attribute

    // Generic static method called as an instance (the CoopMat::Load shape), discarded.
    Loader l;
    l.load<int>(7);
//CHECK: ignoring return value of function declared with '[nodiscard]' attribute

    // Negative cases: a void result and a used result must add no further warning.
    doNothing();
//CHECK-NOT: ignoring return value of function declared with '[nodiscard]' attribute
}
