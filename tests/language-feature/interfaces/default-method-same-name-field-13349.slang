//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// A same-named field does not satisfy a defaulted interface method requirement, so
// conformance falls back to the default, as it does for a same-named method with a
// mismatched signature. This pins current behavior: whether such a member should instead be
// an error is an open question (gh-13353).

interface IFoo
{
    int foo() { return 1; }
}

struct S : IFoo
{
    int foo;
}

int callFoo<T : IFoo>(T t) { return t.foo(); }

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    S s = { 7 };
    // CHECK: 1
    outputBuffer[0] = callFoo(s);
    // CHECK: 7
    outputBuffer[1] = s.foo;
}
