// mutating-method-on-rvalue.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-target hlsl -entry main

struct Counter
{
    int count;

    [mutating] void increment() { count++; }

    void bad()
    {
        increment();
//diag:          ^ mutating method cannot be called on immutable value
//diag:          ^ mutating method 'increment' cannot be called on an immutable value
//diag: ^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    }
}

cbuffer C
{
    Counter gCounter;
}

[shader("compute")]
void main()
{
    gCounter.increment();
//diag:               ^ mutating method cannot be called on immutable value
//diag:               ^ mutating method 'increment' cannot be called on an immutable value
}
