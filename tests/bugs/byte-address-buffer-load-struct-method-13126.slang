// Regression test for shader-slang/slang#13126.
//
// A method call on the result of a `ByteAddressBuffer` load routes the loaded struct through the
// buffer-load-arg specialization pass, which must preserve the load's alignment operand. The
// trigger requires a struct large enough to be deferred-loaded -- `float values[5]` is 20 bytes,
// above the defer-load threshold, whereas `float[4]` at 16 bytes is not -- together with a method
// call on the loaded value. Both `Load` and `LoadAligned` route through the same path.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:set input = ubuffer(data=[1.0 2.0 3.0 4.0 5.0], stride=4)
ByteAddressBuffer input;

//TEST_INPUT:set output = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<float> output;

struct S
{
    float values[5];
    // The accesses span offsets across the 20-byte struct (first and last element).
    float get() { return values[0] + values[4]; }
}

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = input.Load<S>(0).get();
    output[1] = input.LoadAligned<S>(0, 8).get();
    // CHECK: 6.0
    // CHECK: 6.0
}
