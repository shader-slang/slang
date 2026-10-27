//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/methods/hlsl-mutating-method-on-immutable-value.slang -no-codegen

// An HLSL method with an explicit `[mutating]` or `[__ref]` receiver still requires a mutable
// receiver, and calling a `[mutating]` method on an `in` parameter still warns. Only a method
// that uses the HLSL default receiver mode accepts a copy of an immutable receiver, without a
// diagnostic (issue #13964).

struct Counter
{
    int value;

    [mutating]
    void increment() { value++; }

    [__ref]
    void refIncrement() { value++; }

    void defaultIncrement() { value++; }
};

// A call cannot copy a non-copyable receiver, so a default method on such a type still requires a
// mutable receiver.
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

cbuffer C
{
    Counter gCounter;
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
    return param.value;
}

void test()
{
    gCounter.increment();
//CHECK:              ^ mutating method cannot be called on immutable value
//CHECK:              ^ mutating method 'increment' cannot be called on an immutable value
    gCounters[0].increment();
//CHECK:                  ^ mutating method cannot be called on immutable value
//CHECK:                  ^ mutating method 'increment' cannot be called on an immutable value
    makeCounter().increment();
//CHECK:                   ^ mutating method cannot be called on immutable value
//CHECK:                   ^ mutating method 'increment' cannot be called on an immutable value

    gCounter.refIncrement();
//CHECK:                 ^ mutating method cannot be called on immutable value
//CHECK:                 ^ mutating method 'refIncrement' cannot be called on an immutable value

    gCounter.defaultIncrement();
    gCounters[0].defaultIncrement();
    makeCounter().defaultIncrement();

    makeNonCopyableCounter().defaultIncrement();
//CHECK:                                     ^ mutating method cannot be called on immutable value
//CHECK:                                     ^ mutating method 'defaultIncrement' cannot be called on an immutable value
}
