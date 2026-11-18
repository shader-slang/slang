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
    Buffer64<0x100000001ull> buffer64;
    buffer64.data[0] = 1.0;
//CHECK:     ^^^^ error E30034
//CHECK:     ^^^^ array size must not exceed 2147483647.

    Buffer32<0x80000000u> buffer32;
    buffer32.data[0] = 1.0;
//CHECK:     ^^^^ error E30034
//CHECK:     ^^^^ array size must not exceed 2147483647.

    SignedBuffer<-5> signedBuffer;
    signedBuffer.data[0] = 1.0;
//CHECK:         ^^^^ error E30025
//CHECK:         ^^^^ array size must be non-negative.

    // A uint64 size whose magnitude does not fit int64 is still an upper-bound violation,
    // not a negative one.
    Buffer64<0xFFFFFFFFFFFFFFFFull> uint64Max;
    uint64Max.data[0] = 1.0;
//CHECK:      ^^^^ error E30034
//CHECK:      ^^^^ array size must not exceed 2147483647.

    // In-range generic sizes are unaffected. 0x7FFFFFFEu is the largest size this test can
    // use: 0x7FFFFFFF is the unsized-array sentinel (kUnsizedArrayMagicLength) and is rejected
    // by a pre-existing check.
    Buffer64<4ull> buffer64InRange;
    buffer64InRange.data[0] = 1.0;
    Buffer32<0x7FFFFFFEu> buffer32InRange;
    buffer32InRange.data[0] = 1.0;
}
