// This test has been replaced by generated tests that cover all type pair
// combinations. See:
//   tests/generated/generics/gen-generic-type-promotion-inference.slang
//   tests/generated/generics/gen-generic-type-promotion-inference-64bit.slang
//
// To regenerate:
//   python3 extras/test-generators/generate-generic-type-promotion-tests.py
//
// This file retains a minimal smoke test for the basic case.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

T add<T : IArithmetic>(T a, T b)
{
    return a + b;
}

int getInferredTypeCode<T : IArithmetic>(T a, T b)
{
    if (T(-1) > T(0))
        return int(sizeof(T));
    else
        return -int(sizeof(T));
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // int8_t + int32_t: T = int32_t (signed, 4 bytes -> code -4)
    int8_t a = int8_t(3);
    int32_t b = 7;
    outputBuffer[0] = int(add(a, b));
    // CHECK: 10
    outputBuffer[1] = getInferredTypeCode(a, b);
    // CHECK: -4
}
