//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/hlsl-default-this-param-mode-call.slang -no-codegen

// A nonmutating member function cannot call an HLSL-default writable member function, whether the
// receiver is implicit or written as `this`; calls to read-only member functions remain valid.

struct Counter
{
    int value;

    void increment()
    {
        value++;
    }

    [nonmutating]
    int read()
    {
        return value;
    }

    int readTrailingConst() const
    {
        return value;
    }

    [nonmutating]
    int useImplicitThis()
    {
        increment();
//CHECK:         ^ mutating method cannot be called on immutable value
//CHECK:         ^ mutating method 'increment' cannot be called on an immutable value
//CHECK:^^^^^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
        return read() + readTrailingConst();
    }

    [nonmutating]
    int useExplicitThis()
    {
        this.increment();
//CHECK:              ^ mutating method cannot be called on immutable value
//CHECK:              ^ mutating method 'increment' cannot be called on an immutable value
//CHECK:^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
        return this.read() + this.readTrailingConst();
    }

};
