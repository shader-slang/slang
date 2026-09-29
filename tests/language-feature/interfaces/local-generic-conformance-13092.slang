// A generic function-local struct that implements an interface should compile and
// run, with its conformance witness usable through the witness-table paths exercised
// below: a static generic constraint, an existential (dynamic dispatch), and multiple
// interfaces.
// Regression test for https://github.com/shader-slang/slang/issues/13092.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

interface IOp
{
    int apply<T>(T value);
}

interface ITag
{
    int tag();
}

// Dispatches the requirement through a `U : IOp` constraint, so the call must resolve
// through the type's populated conformance witness rather than a direct concrete call.
int viaConstraint<U : IOp>(U u)
{
    return u.apply(1);
}

// Conjunction constraint: exercises that `ensureAllDeclsRec` fills *all* of the inner
// type's witness tables, not only the first conformance.
int viaTwoInterfaces<U : IOp & ITag>(U u)
{
    return u.tag();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    struct Op<V> : IOp, ITag
    {
        V val;
        int apply<T>(T value) { return 3; }
        int tag() { return 7; }
    };

    Op<int> o;
    o.val = 2;

    // Static-constraint dispatch. `o.val` (a generic-typed field) is load-bearing: it
    // is the "non-trivial lowered content" required to reproduce #13092, so it is added
    // into the checked result and must not be dropped when simplifying this test.
    outputBuffer[0] = viaConstraint(o) + o.val; // 3 + 2
    // CHECK: 5

    // Existential / dynamic dispatch -- a distinct lowering entry point on the same
    // now-filled witness table.
    IOp e = o;
    outputBuffer[1] = e.apply(1); // 3
    // CHECK: 3

    // Multiple-interface conformance, dispatched through the conjunction constraint.
    outputBuffer[2] = viaTwoInterfaces(o); // 7
    // CHECK: 7
}
