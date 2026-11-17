// Check that a function called with a struct loaded from a byte-address buffer receives the
// loaded value, for a 144-byte struct without an array and for 20-byte structs that contain an
// array.
//
// Check that this holds whatever the form of the callee (a function, a generic function, a
// function with an interface-typed parameter, a `[noinline]` function, a method), wherever the
// array sits in the struct, for a read-only and for a writable buffer, when one call takes two
// loaded structs, when the argument is a field or an element of a field of the loaded struct,
// and when the loaded value is an array.
//
// Check that a struct loaded from a writable buffer keeps the value that the buffer held when
// the struct was loaded, when the buffer is written between the load and the call.
//
// Check that the shader compiles for SPIR-V, GLSL, Metal and WGSL.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

//TEST_INPUT:set pool = ubuffer(data=[1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40], stride=4)
ByteAddressBuffer pool;

//TEST_INPUT:set rwpool = ubuffer(data=[10 20 30 40 50], stride=4)
RWByteAddressBuffer rwpool;

//TEST_INPUT:set outb = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<uint> outb;

// The interface through which `useGeneric` and `useInterface` read a loaded struct.
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

// The offset of `payload` depends on `T`.
struct Pair<T>
{
    T pad;
    M payload;
}

// A struct with an array of `M`, so that the argument of a call can be one element of a field.
struct Container
{
    uint header;
    M items[2];
}

// Each function combines the first and the last word of its arguments, so that a load at a wrong
// offset or of a wrong size changes its result.
uint useFree(M m) { return m.a + m.y[3]; }
uint useGeneric<T : ISum>(T v) { return v.sum(); }
uint useInterface(ISum v) { return v.sum(); }
[noinline] uint useNoInline(M m) { return m.a + m.y[3]; }
uint useInterleaved(N n) { return n.a * 10 + n.y[0]; }
uint useNested(Q q) { return q.a * 100 + q.y[1][1]; }
uint useBetween(R r) { return r.a * 1000 + r.y[0] * 100 + r.d; }
uint useLarge(B b) { return b.v0.x + b.v8.w; }
[noinline] uint useTwo(M first, M second) { return first.a * 100 + second.y[3]; }
[noinline] uint useArray(uint values[5]) { return values[0] * 100 + values[4]; }

// SPIRV: OpEntryPoint GLCompute %computeMain
// GLSL: void main()
// METAL: void computeMain(
// WGSL: fn computeMain(
[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    outb[0] = useFree(pool.Load<M>(0));
    outb[1] = useGeneric(pool.Load<M>(4));
    outb[2] = useInterface(pool.Load<M>(8));
    outb[3] = useNoInline(pool.Load<M>(12));
    outb[4] = useFree(rwpool.Load<M>(0));
    outb[5] = useInterleaved(pool.Load<N>(0));
    outb[6] = useNested(pool.Load<Q>(0));
    outb[7] = useBetween(pool.Load<R>(4));
    outb[8] = useLarge(pool.Load<B>(0));
    outb[9] = useLarge(pool.LoadAligned<B>(16, 16));

    // A method called on the loaded struct.
    outb[10] = pool.Load<M>(16).sum();

    // Two loaded structs in one call.
    outb[11] = useTwo(pool.Load<M>(id.x + 20), pool.Load<M>(id.x + 8));

    // The same field of two structs of different types, loaded at the same offset.
    outb[12] = useNoInline(pool.Load<Pair<uint>>(id.x + 4).payload);
    outb[13] = useNoInline(pool.Load<Pair<uint4>>(id.x + 4).payload);

    // An element of an array field of the loaded struct.
    outb[14] = useNoInline(pool.LoadAligned<Container>(id.x * 44 + 8, 4).items[id.x + 1]);

    // A loaded array.
    outb[15] = useArray(pool.Load<uint[5]>(id.x + 8));

    // A write to the buffer between the load and the call.
    M saved = rwpool.Load<M>(0);
    rwpool.Store<uint>(0, 1000);
    outb[16] = useNoInline(saved);
    outb[17] = rwpool.Load<uint>(0);

    // CHECK: 6
    // CHECK-NEXT: 8
    // CHECK-NEXT: 10
    // CHECK-NEXT: 12
    // CHECK-NEXT: 60
    // CHECK-NEXT: 51
    // CHECK-NEXT: 105
    // CHECK-NEXT: 2306
    // CHECK-NEXT: 37
    // CHECK-NEXT: 45
    // CHECK-NEXT: 14
    // CHECK-NEXT: 607
    // CHECK-NEXT: 10
    // CHECK-NEXT: 16
    // CHECK-NEXT: 22
    // CHECK-NEXT: 307
    // CHECK-NEXT: 60
    // CHECK-NEXT: 1000
}
