//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// An array type stores its element count as an `int`. A size that comes from a generic value
// parameter is symbolic when the array type is formed, and the count's `int` cast folds when
// the generic argument is substituted. A substituted size above the largest valid size used
// to wrap silently; the checker rejects it when a reference to the specialized field is
// type-checked, once per reference.

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

// A user-written narrowing cast on the count is rejected with the same error: the cast is
// part of the size, not a way to shrink it.
struct CastBuffer<let N : uint64_t>
{
    float data[int(N)];
}

struct Outer<let M : uint64_t>
{
    Buffer64<M> inner;
}

// An inner dimension of a multi-dimensional array is a substituted count like any other.
struct Grid<let N : uint64_t>
{
    float cells[4][N];
}

// A specialization whose oversized field is never referenced in checked code is not
// diagnosed; the check runs when a reference to the field is type-checked.
uniform Buffer64<0x100000001ull> neverUsed;

uniform Buffer64<0x7FFFFFFFull> sentinelUniform;

void fill<let N : uint64_t>(float data[N])
{
    data[0] = 1.0;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Buffer64<0x100000001ull> buffer64;
    buffer64.data[0] = 1.0;
//CHECK:     ^^^^ error E30034
//CHECK:     ^^^^ array size must not exceed 2147483646.
    buffer64.data[1] = 2.0;
//CHECK:     ^^^^ error E30034
//CHECK:     ^^^^ array size must not exceed 2147483646.

    Buffer32<0x80000000u> buffer32;
    buffer32.data[0] = 1.0;
//CHECK:     ^^^^ error E30034
//CHECK:     ^^^^ array size must not exceed 2147483646.

    SignedBuffer<-5> signedBuffer;
    signedBuffer.data[0] = 1.0;
//CHECK:         ^^^^ error E30025
//CHECK:         ^^^^ array size must be non-negative.

    // A positive signed size above the limit is an upper-bound violation too.
    SignedBuffer<0x100000001> signedBig;
    signedBig.data[0] = 1.0;
//CHECK:      ^^^^ error E30034
//CHECK:      ^^^^ array size must not exceed 2147483646.

    // A uint64 size whose magnitude does not fit int64 is still an upper-bound violation,
    // not a negative one.
    Buffer64<0xFFFFFFFFFFFFFFFFull> uint64Max;
    uint64Max.data[0] = 1.0;
//CHECK:      ^^^^ error E30034
//CHECK:      ^^^^ array size must not exceed 2147483646.

    // 0x7FFFFFFF is the unsized-array sentinel, so a size of exactly that value would
    // silently become an unsized array; the check rejects it.
    sentinelUniform.data[0] = 1.0;
//CHECK:            ^^^^ error E30034
//CHECK:            ^^^^ array size must not exceed 2147483646.

    CastBuffer<0x100000001ull> castBuffer;
    castBuffer.data[0] = 1.0;
//CHECK:       ^^^^ error E30034
//CHECK:       ^^^^ array size must not exceed 2147483646.

    Outer<0x100000001ull> outer;
    outer.inner.data[0] = 1.0;
//CHECK:        ^^^^ error E30034
//CHECK:        ^^^^ array size must not exceed 2147483646.

    Grid<0x100000001ull> grid;
    grid.cells[0][1] = 1.0;
//CHECK: ^^^^^ error E30034
//CHECK: ^^^^^ array size must not exceed 2147483646.

    fill<0x100000001ull>(buffer64.data);
//CHECK:                          ^^^^ error E30034
//CHECK:                          ^^^^ array size must not exceed 2147483646.

    // In-range generic sizes are unaffected. 0x7FFFFFFE is the largest valid size; the
    // sentinel value above it is rejected.
    Buffer64<4ull> buffer64InRange;
    buffer64InRange.data[0] = 1.0;
    Buffer32<0x7FFFFFFEu> buffer32InRange;
    buffer32InRange.data[0] = 1.0;
}
