// Regression test for shader-slang/slang#13126: a `ByteAddressBuffer` load passed as a call argument.
//
// Any call that takes the loaded struct as an argument routes the load through buffer-load-arg
// specialization once the struct is deferred-loaded (larger than 16 bytes and containing an array),
// so the specialized callee must receive the load with its alignment operand intact. The call need
// not be a method: a free function, a generic, an interface-typed parameter and a `[noinline]`
// callee all take this path, from both `ByteAddressBuffer` and `RWByteAddressBuffer`, and for any
// position of the array within the struct.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=CHECKO0):-target spirv-asm -entry computeMain -stage compute -O0

//TEST_INPUT:set pool = ubuffer(data=[1 2 3 4 5 6 7 8 9 10], stride=4)
ByteAddressBuffer pool;

//TEST_INPUT:set rwpool = ubuffer(data=[10 20 30 40 50], stride=4)
RWByteAddressBuffer rwpool;

//TEST_INPUT:set outb = out ubuffer(data=[0 0 0 0 0 0 0], stride=4)
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

uint useFree(M m) { return m.a + m.y[3]; }
uint useGeneric<T : ISum>(T v) { return v.sum(); }
uint useInterface(ISum v) { return v.sum(); }
[noinline] uint useNoInline(M m) { return m.a + m.y[3]; }
uint useInterleaved(N n) { return n.a * 10 + n.y[0]; }
uint useNested(Q q) { return q.a * 100 + q.y[1][1]; }

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECKO0: OpEntryPoint
    outb[0] = useFree(pool.Load<M>(0));
    outb[1] = useGeneric(pool.Load<M>(4));
    outb[2] = useInterface(pool.Load<M>(8));
    outb[3] = useNoInline(pool.Load<M>(12));
    outb[4] = useFree(rwpool.Load<M>(0));
    outb[5] = useInterleaved(pool.Load<N>(0));
    outb[6] = useNested(pool.Load<Q>(0));
    // CHECK: 6
    // CHECK-NEXT: 8
    // CHECK-NEXT: 10
    // CHECK-NEXT: 12
    // CHECK-NEXT: 60
    // CHECK-NEXT: 51
    // CHECK-NEXT: 105
}
