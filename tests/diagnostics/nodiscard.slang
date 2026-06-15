//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for shader-slang/slang#11454.
// A `[NoDiscard]`-marked function whose non-void result is discarded at a
// top-level statement must produce an error (E30059). Target-independent: this
// is a front-end check, so it needs no GPU/target.
//
// Coverage spans the shapes the feature is actually deployed against — a
// generic static method (the `CoopMat::Load` family) and the
// static-method-called-as-instance form — plus negative cases (a void result
// and a used result) which must NOT error.

[NoDiscard]
int computeValue()
{
    return 42;
}

[NoDiscard]
T genericCompute<T>(T x)
{
    return x;
}

struct Loader
{
    // Generic static method — the same shape as `CoopMat::Load<...>`.
    [NoDiscard]
    static T load<T>(T x)
    {
        return x;
    }
}

// `[NoDiscard]` on a void-returning function: there is no result to discard,
// so a discarded call must NOT error.
[NoDiscard]
void doNothing()
{
}

// Using the result (returning it) must NOT error.
int useResult()
{
    return computeValue();
}

void test()
{
    // Plain free function, discarded.
    computeValue();
//CHECK: error{{.*}}ignoring return value of function declared with '[NoDiscard]' attribute

    // Generic free function, discarded.
    genericCompute<int>(3);
//CHECK: error{{.*}}ignoring return value of function declared with '[NoDiscard]' attribute

    // Generic static method, discarded.
    Loader.load<int>(5);
//CHECK: error{{.*}}ignoring return value of function declared with '[NoDiscard]' attribute

    // Generic static method called as an instance (the CoopMat::Load shape), discarded.
    Loader l;
    l.load<int>(7);
//CHECK: error{{.*}}ignoring return value of function declared with '[NoDiscard]' attribute

    // Negative cases: a void result and a used result must add no further error.
    doNothing();
//CHECK-NOT: ignoring return value of function declared with '[NoDiscard]' attribute
}
