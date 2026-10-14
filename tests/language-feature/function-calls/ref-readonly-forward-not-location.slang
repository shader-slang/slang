// A `__ref_readonly` argument must still be a memory location, and a read-only location is still
// not writable.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

struct S { int a; }

int leaf(__ref_readonly int x) { return x; }

int forward(__ref_readonly S s)
{
    s.a = 1;
//CHECK:^ E30011
//CHECK:^ left of '=' is not an l-value.
    return leaf(s.a + 1);
//CHECK:            ^ E30047
//CHECK:            ^ argument passed to parameter '0' must be l-value.
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    let value = 3;
    outputBuffer[0] = leaf(1) + leaf(value);
//CHECK:                   ^ E30047
//CHECK:                   ^ argument passed to parameter '0' must be l-value.
//CHECK:                             ^^^^^ E30047
//CHECK:                             ^^^^^ argument passed to parameter '0' must be l-value.
}
