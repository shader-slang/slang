// Explicitly specializing a generic interface method with an interface type must be rejected:
// the argument `IFoo` denotes the existential `dyn IFoo`, which does not conform to `IFoo`
// (shader-slang/slang#12430).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -conformance "B:IBar" -conformance "Foo:IFoo" -conformance "AnotherFoo:IFoo"
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain -conformance "B:IBar" -conformance "Foo:IFoo" -conformance "AnotherFoo:IFoo"
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target glsl -stage compute -entry computeMain -conformance "B:IBar" -conformance "Foo:IFoo" -conformance "AnotherFoo:IFoo"
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -stage compute -entry computeMain -conformance "B:IBar" -conformance "Foo:IFoo" -conformance "AnotherFoo:IFoo"

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
//CHECK:           ^^^^ error E38029
//CHECK:           ^^^^ type argument 'dyn IFoo' does not conform to the required interface 'IFoo'
    outputBuffer[0] = result;
}
