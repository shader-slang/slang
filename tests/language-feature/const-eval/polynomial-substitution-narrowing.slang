//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// An array size that depends on a generic value parameter is a polynomial in that parameter.
// Substituting a concrete argument combines the terms in 64 bits, so the constant that results
// must be narrowed to the type of the size. For example, with `N = 0xFFFFFFFFu`, the `uint`
// size `(N + 1u) * 2u + 1u` sums to 0x200000001 in 64 bits, and its `uint` value is 1.
// Debug builds assert that every constant is narrowed to its type.

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outBuf
RWStructuredBuffer<int> outBuf;

struct WrapAfterMul<let N : uint>
{
    float data[(N + 1u) * 2u + 1u];
}

struct WrapAfterAdd<let N : uint>
{
    float data[N + 2u];
}

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
