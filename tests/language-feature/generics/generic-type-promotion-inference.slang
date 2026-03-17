// Test that ambiguous generic type inference is resolved by promotion rank.
// When a generic parameter is inferred from arguments of different scalar widths,
// inference prefers candidates with lower conversion cost (e.g., wider
// same-kind integer promotions are favored over narrowing conversions).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

T add<T : IArithmetic>(T a, T b)
{
    return a + b;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // int8_t + int32_t: T should infer as int32_t
    int8_t a = int8_t(10);
    int32_t b = 20;
    outputBuffer[0] = int(add(a, b));
    // CHECK: 30

    // int16_t + int32_t: T should promote to int32_t
    int16_t c = int16_t(100);
    int32_t d = 200;
    outputBuffer[1] = int(add(c, d));
    // CHECK: 300

    // uint8_t + uint32_t: T should promote to uint32_t
    uint8_t e = uint8_t(15);
    uint32_t f = 25u;
    outputBuffer[2] = int(add(e, f));
    // CHECK: 40
}
