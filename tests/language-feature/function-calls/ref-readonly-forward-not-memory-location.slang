// A `__ref_readonly` argument must still be a memory location, and a read-only location is still
// not writable. A swizzle that repeats a component does not name a memory location.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

struct S { int a; int4 v; }

int leaf(__ref_readonly int x) { return x; }
int leaf2(__ref_readonly int2 x) { return x.x; }

int forward(__ref_readonly S s)
{
    s.a = 1;
//CHECK:^ E30011
//CHECK:^ left of '=' is not an l-value.
    leaf2(s.v.xx);
//CHECK:      ^^ E30047
//CHECK:      ^^ argument passed to parameter '0' must be l-value.
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
