// THIS IS A GENERATED FILE. DO NOT EDIT!
// Instead, edit extras/test-generators/generate-generic-type-promotion-tests.py
// and regenerate by running:
//
//     python3 extras/test-generators/generate-generic-type-promotion-tests.py
//
// Generic type promotion inference: 64-bit scalar types
// Tests 24 type pair combinations.
//
// Each case validates both the arithmetic result (add) and the inferred type
// identity (getInferredTypeCode). The type code encodes sizeof(T) with sign:
//   positive = unsigned type, negative = signed or float type.
//
// Bool promotion is not tested due to non-deterministic inference behavior
// tracked in issue #10164.
//
// Excluded pairs: integer+half (E30081, CPU backend limitation) and
// float+double (E30082 performance warning).
//
// 64-bit types (int64_t, uint64_t, double) may require extensions on GPU
// backends, so this test runs on CPU only.

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

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int idx = 0;

    // Case 0: int8_t + int64_t -> T = int64_t (code -8)
    {
        int8_t a = int8_t(3);
        int64_t b = int64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 1: int8_t + uint64_t -> T = uint64_t (code 8)
    {
        int8_t a = int8_t(3);
        uint64_t b = uint64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 2: int8_t + double -> T = double (code -8)
    {
        int8_t a = int8_t(3);
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 3: int16_t + int64_t -> T = int64_t (code -8)
    {
        int16_t a = int16_t(3);
        int64_t b = int64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 4: int16_t + uint64_t -> T = uint64_t (code 8)
    {
        int16_t a = int16_t(3);
        uint64_t b = uint64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 5: int16_t + double -> T = double (code -8)
    {
        int16_t a = int16_t(3);
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 6: int32_t + int64_t -> T = int64_t (code -8)
    {
        int32_t a = 3;
        int64_t b = int64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 7: int32_t + uint64_t -> T = uint64_t (code 8)
    {
        int32_t a = 3;
        uint64_t b = uint64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 8: int32_t + double -> T = double (code -8)
    {
        int32_t a = 3;
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 9: uint8_t + int64_t -> T = int64_t (code -8)
    {
        uint8_t a = uint8_t(3);
        int64_t b = int64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 10: uint8_t + uint64_t -> T = uint64_t (code 8)
    {
        uint8_t a = uint8_t(3);
        uint64_t b = uint64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 11: uint8_t + double -> T = double (code -8)
    {
        uint8_t a = uint8_t(3);
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 12: uint16_t + int64_t -> T = int64_t (code -8)
    {
        uint16_t a = uint16_t(3);
        int64_t b = int64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 13: uint16_t + uint64_t -> T = uint64_t (code 8)
    {
        uint16_t a = uint16_t(3);
        uint64_t b = uint64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 14: uint16_t + double -> T = double (code -8)
    {
        uint16_t a = uint16_t(3);
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 15: uint32_t + int64_t -> T = int64_t (code -8)
    {
        uint32_t a = 3u;
        int64_t b = int64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 16: uint32_t + uint64_t -> T = uint64_t (code 8)
    {
        uint32_t a = 3u;
        uint64_t b = uint64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 17: uint32_t + double -> T = double (code -8)
    {
        uint32_t a = 3u;
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 18: half + double -> T = double (code -8)
    {
        half a = half(3);
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 19: float + int64_t -> T = float (code -4)
    {
        float a = float(3);
        int64_t b = int64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -4
    }

    // Case 20: float + uint64_t -> T = float (code -4)
    {
        float a = float(3);
        uint64_t b = uint64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -4
    }

    // Case 21: int64_t + uint64_t -> T = uint64_t (code 8)
    {
        int64_t a = int64_t(3);
        uint64_t b = uint64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 22: int64_t + double -> T = double (code -8)
    {
        int64_t a = int64_t(3);
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 23: uint64_t + double -> T = double (code -8)
    {
        uint64_t a = uint64_t(3);
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }
}
