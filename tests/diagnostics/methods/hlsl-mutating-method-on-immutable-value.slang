//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/methods/hlsl-mutating-method-on-immutable-value.slang -no-codegen

// In HLSL code, a call can pass a copy of an immutable receiver argument to an `inout` `this`
// parameter, whether the callee's `inout` mode is the HLSL default or an explicit `[mutating]`
// (issue #13964). A `[__ref]` `this` parameter and a non-copyable receiver still require a mutable
// receiver argument. A call on a legacy `in` parameter still warns that the callee's writes stay
// in the parameter.

struct Counter
{
    int value;

    [mutating]
    void increment() { value++; }

    [__ref]
    void refIncrement() { value++; }

    void defaultIncrement() { value++; }
};

[__NonCopyableType]
struct NonCopyableCounter
{
    int value;

    void defaultIncrement() { value++; }
};

NonCopyableCounter makeNonCopyableCounter()
{
    NonCopyableCounter counter;
    counter.value = 0;
    return counter;
}

// Lookup of `bump` on these types also finds the requirement through the conformance. A call
// through it runs the declaration that the requirement resolves to: the `[mutating]` method itself,
// or, for the `[__ref]` method, a synthesized `inout` wrapper that calls it.
interface ICounter
{
    int bump();
};

struct InterfaceCounter : ICounter
{
    int value;

    [mutating]
    int bump() { return ++value; }
};

struct RefInterfaceCounter : ICounter
{
    int value;

    [__ref]
    int bump() { return ++value; }
};

cbuffer C
{
    Counter gCounter;
    InterfaceCounter gInterfaceCounter;
    RefInterfaceCounter gRefInterfaceCounter;
};

StructuredBuffer<Counter> gCounters;

Counter makeCounter()
{
    Counter counter;
    counter.value = 0;
    return counter;
}

int useParam(Counter param)
{
    param.increment();
//CHECK:           ^ mutating method called on `in` parameter
//CHECK:           ^ mutating method 'increment' called on `in` parameter 'param'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
    param.defaultIncrement();
//CHECK:                  ^ mutating method called on `in` parameter
//CHECK:                  ^ mutating method 'defaultIncrement' called on `in` parameter 'param'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended
    return param.value;
}

void test()
{
    gCounter.increment();
    gCounters[0].increment();
    makeCounter().increment();
    gCounter.defaultIncrement();
    gCounters[0].defaultIncrement();
    makeCounter().defaultIncrement();
    gInterfaceCounter.bump();
    gRefInterfaceCounter.bump();

    gCounter.refIncrement();
//CHECK:                 ^ mutating method cannot be called on immutable value
//CHECK:                 ^ mutating method 'refIncrement' cannot be called on an immutable value

    makeNonCopyableCounter().defaultIncrement();
//CHECK:                                     ^ mutating method cannot be called on immutable value
//CHECK:                                     ^ mutating method 'defaultIncrement' cannot be called on an immutable value
}
