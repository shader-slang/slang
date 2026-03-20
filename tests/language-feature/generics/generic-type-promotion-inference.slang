// Test that generic type inference picks the correct unified type when
// arguments have different scalar widths. Each case verifies both the
// arithmetic result and the inferred type identity (via getInferredTypeCode).
//
// The default (DX) backend test line is omitted because sub-word integer
// types (int8_t, int16_t, uint8_t) have limited DX backend support.
//
// Bool promotion is not tested here due to non-deterministic inference
// behavior tracked in issue #10164.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

T add<T : IArithmetic>(T a, T b)
{
    return a + b;
}

// Returns a code identifying the inferred type T:
//   sizeof(T)  for unsigned types  (T(-1) > T(0))
//  -sizeof(T)  for signed / float types
int getInferredTypeCode<T : IArithmetic>(T a, T b)
{
    if (T(-1) > T(0))
        return int(sizeof(T));
    else
        return -int(sizeof(T));
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int idx = 0;

    // int8_t + int32_t: T = int32_t (signed, 4 bytes -> code -4)
    int8_t a = int8_t(10);
    int32_t b = 20;
    int32_t r0 = add(a, b);
    outputBuffer[idx++] = r0;
    // CHECK: 30
    outputBuffer[idx++] = getInferredTypeCode(a, b);
    // CHECK: -4

    // int16_t + int32_t: T = int32_t (signed, 4 bytes -> code -4)
    int16_t c = int16_t(100);
    int32_t d = 200;
    int32_t r1 = add(c, d);
    outputBuffer[idx++] = r1;
    // CHECK: 300
    outputBuffer[idx++] = getInferredTypeCode(c, d);
    // CHECK: -4

    // uint8_t + uint32_t: T = uint32_t (unsigned, 4 bytes -> code 4)
    uint8_t e = uint8_t(15);
    uint32_t f = 25u;
    uint32_t r2 = add(e, f);
    outputBuffer[idx++] = int(r2);
    // CHECK: 40
    outputBuffer[idx++] = getInferredTypeCode(e, f);
    // CHECK: 4

    // int8_t + uint32_t: mixed signed/unsigned with negative value
    // T = uint32_t (unsigned, 4 bytes -> code 4)
    int8_t g = int8_t(-3);
    uint32_t h = 10u;
    uint32_t r3 = add(g, h);
    outputBuffer[idx++] = int(r3);
    // CHECK: 7
    outputBuffer[idx++] = getInferredTypeCode(g, h);
    // CHECK: 4

    // half + float: T = float (4 bytes -> code -4)
    half i = half(1.5);
    float j = 3.5;
    float r4 = add(i, j);
    outputBuffer[idx++] = int(r4);
    // CHECK: 5
    outputBuffer[idx++] = getInferredTypeCode(i, j);
    // CHECK: -4
}
