//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// A conforming type may declare a field named like an interface method that has a default
// body. The field cannot satisfy the requirement, so conformance falls back to the default
// (gh-13349; previously this aborted the compiler during witness synthesis).

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
