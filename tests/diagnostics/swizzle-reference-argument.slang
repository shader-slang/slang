// A reference parameter needs its argument to be a memory location, and the front end decides
// whether a swizzle is one from its components alone, whatever the access of its base:
//
// - With a repeated component (`v.xx`), a swizzle is a pure value.
// - With distinct components that are not adjacent and in order (`v.xz`), it is a storage
//   location that can be written component by component, but it has no single address.
// - With adjacent components in order (`v.yz`), it is a memory location.
// - A multi-component matrix swizzle is never a memory location, because the elements of a
//   matrix need not be adjacent.
//
// The first two kinds can still be passed to an `inout` parameter, which writes back through
// the storage location rather than referring to it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute -experimental-feature

void rw2(__ref int2 x) { x += 1; }
int2 ro2(__ref_readonly int2 x) { return x; }
void io2(inout int2 x) { x += 1; }

void fromWritable(inout int4 v, inout int4x4 m)
{
    rw2(v.xx);
//CHECK:  ^^ E30047
//CHECK:  ^^ argument passed to parameter '0' must be l-value.
    rw2(v.xz);
//CHECK:  ^^ E30047
//CHECK:  ^^ argument passed to parameter '0' must be l-value.
    rw2(m._m00_m01);
//CHECK:  ^^^^^^^^ E30047
//CHECK:  ^^^^^^^^ argument passed to parameter '0' must be l-value.
    io2(v.xz);
    io2(m._m00_m01);
}

int2 fromReadOnly(__ref_readonly int4 v)
{
    ro2(v.xx);
//CHECK:  ^^ E30047
//CHECK:  ^^ argument passed to parameter '0' must be l-value.
    ro2(v.xz);
//CHECK:  ^^ E30047
//CHECK:  ^^ argument passed to parameter '0' must be l-value.
    return v.xy;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    int4 v = 0;
    int4x4 m = 0;
    fromWritable(v, m);
    outputBuffer[0] = fromReadOnly(v).x;
}
