// Explicitly specializing a generic interface method with an interface
// type should be rejected with the same existential-specialization
// diagnostic as the implicit form. Previously this crashed in typeflow.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain -conformance "B:IBar" -conformance "Foo:IFoo" -conformance "AnotherFoo:IFoo"

#lang slang 2025

interface IFoo
{
    void doSomething();
}

struct Foo : IFoo
{
    void doSomething() {}
}

struct AnotherFoo : IFoo
{
    void doSomething() {}
}

interface IBar
{
    int eval<T : IFoo>(T t);
}

export struct B : IBar
{
    int eval<T : IFoo>(T t)
    {
        return 0;
    }
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    IBar b = createDynamicObject<IBar>(id, 0);
    IFoo df = createDynamicObject<IFoo>(id, 0);
    int result = b.eval<IFoo>(df);
//CHECK:                     ^ cannot specialize generic with existential type
//CHECK:                     ^ specializing 'B.eval' with an existential type is not allowed.
    outputBuffer[0] = result;
}
