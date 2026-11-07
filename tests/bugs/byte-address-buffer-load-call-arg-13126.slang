// Regression test for shader-slang/slang#13126: a `ByteAddressBuffer` load passed as a call argument.
//
// A struct is deferred-loaded when it is larger than 128 bytes, or larger than 16 bytes and contains
// an array. A call that takes such a struct as an argument has the load rebuilt inside a specialized
// callee by buffer-load-arg specialization. The rebuilt load must keep its alignment operand whatever the callee's form and
// wherever the array sits in the struct.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=CHECKO0):-target spirv-asm -entry computeMain -stage compute -O0

//TEST_INPUT:set pool = ubuffer(data=[1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40], stride=4)
ByteAddressBuffer pool;

//TEST_INPUT:set rwpool = ubuffer(data=[10 20 30 40 50], stride=4)
RWByteAddressBuffer rwpool;

//TEST_INPUT:set outb = out ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<uint> outb;

interface ISum
{
    uint sum();
}

// 20 bytes, array after a scalar.
struct M : ISum
{
    uint a;
    uint y[4];
    uint sum() { return a + y[3]; }
}

// 20 bytes, array before a scalar.
struct N
{
    uint y[4];
    uint a;
}

// 20 bytes, nested array.
struct Q
{
    uint a;
    uint y[2][2];
}

// 20 bytes, array between scalars.
struct R
{
    uint a;
    uint y[1];
    uint b, c, d;
}

// 144 bytes, no array.
struct B
{
    uint4 v0, v1, v2, v3, v4, v5, v6, v7, v8;
}

uint useFree(M m) { return m.a + m.y[3]; }
uint useGeneric<T : ISum>(T v) { return v.sum(); }
uint useInterface(ISum v) { return v.sum(); }
[noinline] uint useNoInline(M m) { return m.a + m.y[3]; }
uint useInterleaved(N n) { return n.a * 10 + n.y[0]; }
uint useNested(Q q) { return q.a * 100 + q.y[1][1]; }
uint useBetween(R r) { return r.a * 1000 + r.y[0] * 100 + r.d; }
uint useLarge(B b) { return b.v0.x + b.v8.w; }

[numthreads(1, 1, 1)]
void computeMain()
{
    // The -O0 arm only checks that compilation completes.
    // CHECKO0: OpEntryPoint
    outb[0] = useFree(pool.Load<M>(0));
    outb[1] = useGeneric(pool.Load<M>(4));
    outb[2] = useInterface(pool.Load<M>(8));
    outb[3] = useNoInline(pool.Load<M>(12));
    outb[4] = useFree(rwpool.Load<M>(0));
    outb[5] = useInterleaved(pool.Load<N>(0));
    outb[6] = useNested(pool.Load<Q>(0));
    outb[7] = useBetween(pool.Load<R>(4));
    outb[8] = useLarge(pool.Load<B>(0));
    // CHECK: 6
    // CHECK-NEXT: 8
    // CHECK-NEXT: 10
    // CHECK-NEXT: 12
    // CHECK-NEXT: 60
    // CHECK-NEXT: 51
    // CHECK-NEXT: 105
    // CHECK-NEXT: 2306
    // CHECK-NEXT: 37
}
