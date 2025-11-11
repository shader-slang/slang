//TEST:SIMPLE(filecheck=CHECK): -target hlsl

#lang slang 2025

interface IFoo { void doSomething();}
struct Foo : IFoo{
    void doSomething() {}
}

struct SO<T:IFoo>
{
    static const int svalue = 2;
    [mutating]
    int call()
    {
        if (svalue > 0)
            return svalue;
        return 0;
    }
}

interface IBar
{
    int eval<T:IFoo>(T t);
}

export struct B : IBar
{
    int eval<T:IFoo>(T t)
    {
        SO<T> so = {};
        return so.call();
    }
}
RWStructuredBuffer<int> outputBuffer;
[numthreads(1,1,1)]
void computeMain(int id : SV_DispatchThreadID)
{
    IBar b = createDynamicObject<IBar>(id, 0);

    Foo foo = {};
    IFoo df = createDynamicObject<IFoo>(id, 0);
    // this creates a `specialize(lookupwitness(w, "eval"))`
    // and we need to make sure we can also diagnose properly.
    // CHECK: ([[@LINE+1]]): error 33180:
    int result = b.eval(df);
    outputBuffer[0] = result;
}