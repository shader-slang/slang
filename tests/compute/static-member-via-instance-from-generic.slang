// Negative regression test for shader-slang/slang#11004: a plain
// `static` member function called via an instance expression from
// inside a generic body reaches the same `StaticMemberExpr`-with-
// `SharedTypeExpr`-base lowering path the AD 2.0 receiver-threading
// fix touches, but must NOT synthesize an implicit `this` argument.
// The fix's gate keys on `asLookupOnCallable(declRefBase)` so this
// shape stays untouched; if a future broadening of that predicate
// over-fires here, the call would gain a spurious `this` arg and
// downstream IR-funcType-arity checks would trip.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)

interface IBase
{
    static float helper(float t);
}

struct Foo : IBase
{
    static float helper(float t)
    {
        return 2.0 * t;
    }
}

float runStatic<T : IBase>(T obj, float t)
{
    return obj.helper(t);
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Foo f;

    // CHECK: 4.0
    outputBuffer[0] = runStatic<Foo>(f, 2.0);

    // CHECK: 6.0
    outputBuffer[1] = runStatic<Foo>(f, 3.0);
}
