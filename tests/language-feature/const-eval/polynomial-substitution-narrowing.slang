//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// An array size that depends on a generic value parameter is a polynomial in that parameter.
// Substituting a concrete argument combines the terms in 64 bits, so the constant that results
// must be narrowed to the type of the size. For example, with `N = 0xFFFFFFFFu`, the `uint`
// size `(N + 1u) * 2u + 1u` sums to 0x200000001 in 64 bits, and its `uint` value is 1.
// Debug builds assert that every constant is narrowed to its type.

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outBuf
RWStructuredBuffer<int> outBuf;

// With `N = 0xFFFFFFFFu`, `N + 1u` wraps to 0 in `uint`, so the size is 0 * 2 + 1 = 1. In 64 bits
// it would be 0x200000001.
struct WrapAfterMul<let N : uint>
{
    float data[(N + 1u) * 2u + 1u];
}

// With `N = 0xFFFFFFFFu`, `N + 2u` wraps to 1 in `uint`. In 64 bits it would be 0x100000001.
struct WrapAfterAdd<let N : uint>
{
    float data[N + 2u];
}

// With `N = 1u`, the term `N * 65536u * 65536u` is 2^32, which wraps to 0 in `uint`, so the size
// is 3. In 64 bits it would be 0x100000003.
struct WrapHighTerm<let N : uint>
{
    float data[N * 65536u * 65536u + 3u];
}

[numthreads(1, 1, 1)]
void computeMain()
{
    WrapAfterMul<0xFFFFFFFFu> a;
    WrapAfterAdd<0xFFFFFFFFu> b;
    WrapHighTerm<1u> c;
    outBuf[0] = a.data.getCount();
    outBuf[1] = b.data.getCount();
    outBuf[2] = c.data.getCount();
}

// CHECK: type: int32_t
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}3{{$}}
