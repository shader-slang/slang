//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E33180 (cannot-specialize-generic-with-existential) - cannot specialize generic with existential type
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=33180
//META: catalog_name=cannot-specialize-generic-with-existential
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Calling a generic from inside a dynamic-dispatch interface
// method with an existential argument forces specialization of
// a user-defined generic with an existential, which is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -conformance "B:IBar" -conformance "Foo:IFoo" -stage compute -entry computeMain

#lang slang 2025

interface IFoo { void doSomething(); }
struct Foo : IFoo { void doSomething() {} }

struct SO<T:IFoo>
{
    static const int v = 2;
    [mutating]
    int call() { return v; }
}

interface IBar { int eval<T:IFoo>(T t); }

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
    IFoo df = createDynamicObject<IFoo>(id, 0);
    outputBuffer[0] = b.eval(df);
}

// CHECK: E33180
