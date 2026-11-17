// A `__ref_readonly` argument must still be a memory location, and a read-only location is still
// not writable. A swizzle names a memory location only if its components are adjacent and in
// order, so `v.xx`, `v.xz`, and `v.yx` do not. Neither does a property, whose getter returns a
// temporary. A matrix or tuple element of a read-only location is itself read-only, while a swizzle
// of several matrix or tuple elements does not name a memory location.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute -experimental-feature

struct S { int a; int4 v; property int p { get { return a; } } }

int leaf(__ref_readonly int x) { return x; }
int leaf2(__ref_readonly int2 x) { return x.x; }
void write(__ref int x) { x = 1; }

int forwardMatrix(__ref_readonly int4x4 m)
{
    m._m00 = 1;
//CHECK:   ^ E30011
//CHECK:   ^ left of '=' is not an l-value.
    write(m._m11);
//CHECK:    ^^^^ E30047
//CHECK:    ^^^^ argument passed to parameter '0' must be l-value.
    return m[0][0];
}

int forwardTuple(__ref_readonly Tuple<int, int> t)
{
    t._0 = 1;
//CHECK: ^ E30011
//CHECK: ^ left of '=' is not an l-value.
    return t._1;
}

int leafTuple(__ref_readonly Tuple<int, int> x) { return x._0; }

int forwardMatrixSwizzle(__ref_readonly int4x4 m)
{
    return leaf2(m._m00_m01);
//CHECK:           ^^^^^^^^ E30047
//CHECK:           ^^^^^^^^ argument passed to parameter '0' must be l-value.
}

int forwardTupleSwizzle(__ref_readonly Tuple<int, int, int> t)
{
    return leafTuple(t._0_1);
//CHECK:               ^^^^ E30047
//CHECK:               ^^^^ argument passed to parameter '0' must be l-value.
}

int forward(__ref_readonly S s)
{
    leaf2(s.v.xz);
//CHECK:      ^^ E30047
//CHECK:      ^^ argument passed to parameter '0' must be l-value.
    leaf2(s.v.yx);
//CHECK:      ^^ E30047
//CHECK:      ^^ argument passed to parameter '0' must be l-value.
    s.a = 1;
//CHECK:^ E30011
//CHECK:^ left of '=' is not an l-value.
    leaf2(s.v.xx);
//CHECK:      ^^ E30047
//CHECK:      ^^ argument passed to parameter '0' must be l-value.
    leaf(s.p);
//CHECK:   ^ E30047
//CHECK:   ^ argument passed to parameter '0' must be l-value.
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
