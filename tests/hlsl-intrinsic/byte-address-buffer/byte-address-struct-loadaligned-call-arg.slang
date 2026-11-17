// Check that a method called on a struct loaded from a byte-address buffer with `Load` or
// `LoadAligned` receives the loaded value.
//
// Check that this holds when the alignment passed to `LoadAligned` is a literal, a `static const`
// and a generic value parameter, when the offset and the alignment are the same constant, and for
// the natural-alignment overload `LoadAligned<T>(location)` in an ordinary function and in a
// generic function.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:set input = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0 13.0 14.0 15.0 16.0 17.0 18.0 19.0 20.0 21.0 22.0 23.0 24.0], stride=4)
ByteAddressBuffer input;

//TEST_INPUT:set output = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> output;

// 32 bytes, with an array.
struct S
{
    float4 values[2];
    float get() { return values[0].x * 100.0 + values[1].w; }
}

static const uint kAlignment = 8;

// Returns `get()` of the `S` loaded at `offset`, which is a multiple of `A`.
float loadWithAlignment<let A : uint>(uint offset)
{
    return input.LoadAligned<S>(offset, A).get();
}

// The interface through which `loadWithNaturalAlignment` reads a loaded value.
interface IValue
{
    // Returns a value computed from the fields of `v`.
    static float getValue(This v);
}

extension S : IValue
{
    static float getValue(S v) { return v.get(); }
}

// Returns `getValue` of the `T` loaded at `offset`, which is a multiple of the natural alignment
// of `T`.
float loadWithNaturalAlignment<T : IValue>(uint offset)
{
    return T.getValue(input.LoadAligned<T>(offset));
}

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = input.Load<S>(0).get();
    output[1] = input.LoadAligned<S>(16, 16).get();
    output[2] = input.LoadAligned<S>(16, 4).get();
    output[3] = input.LoadAligned<S>(4, 4).get();
    output[4] = input.LoadAligned<S>(8, kAlignment).get();

    output[5] = loadWithAlignment<8>(24);

    // The natural-alignment overload requires a location that is a multiple of 32 for `S`.
    output[6] = input.LoadAligned<S>(32).get();

    output[7] = loadWithNaturalAlignment<S>(64);

    // CHECK: 108.0
    // CHECK-NEXT: 512.0
    // CHECK-NEXT: 512.0
    // CHECK-NEXT: 209.0
    // CHECK-NEXT: 310.0
    // CHECK-NEXT: 714.0
    // CHECK-NEXT: 916.0
    // CHECK-NEXT: 1724.0
}
