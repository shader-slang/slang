//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// An array type stores its element count as an `int`. A size that comes from a generic value
// parameter is symbolic when the array type is formed, and the generic argument is converted
// to `int` when it is substituted. A substituted size above INT32_MAX used to wrap silently
// (shader-slang/slang#14595); the checker rejects it once the specialized type is used.

struct Buffer64<let N : uint64_t>
{
    float data[N];
}

struct Buffer32<let N : uint>
{
    float data[N];
}

struct SignedBuffer<let N : int64_t>
{
    float data[N];
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Buffer64<0x100000001ull> a;
    a.data[0] = 1.0;
//CHECK: error E30034
//CHECK: array size must not exceed 2147483647.

    Buffer32<0x80000000u> b;
    b.data[0] = 1.0;
//CHECK: error E30034
//CHECK: array size must not exceed 2147483647.

    SignedBuffer<-5> c;
    c.data[0] = 1.0;
//CHECK: error E30025
//CHECK: array size must be non-negative.

    // In-range generic sizes are unaffected.
    Buffer64<4ull> d;
    d.data[0] = 1.0;
    Buffer32<0x7FFFFFFFu> e;
    e.data[0] = 1.0;
}
