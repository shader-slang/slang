//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/hlsl-mutating-default-immutable-receiver.slang -fhlsl-methods-mutating-by-default -no-codegen

// With the opt-in, even a method that only reads its receiver requires a writable base.

struct Counter
{
    int value;

    int read()
    {
        return value;
    }
};

cbuffer C
{
    Counter counter;
};

int readCounter()
{
    return counter.read();
//CHECK:               ^ mutating method cannot be called on immutable value
//CHECK:               ^ mutating method 'read' cannot be called on an immutable value
}
