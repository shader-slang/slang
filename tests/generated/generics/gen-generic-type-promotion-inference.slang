// THIS IS A GENERATED FILE. DO NOT EDIT!
// Instead, edit extras/test-generators/generate-generic-type-promotion-tests.py
// and regenerate by running:
//
//     python3 extras/test-generators/generate-generic-type-promotion-tests.py
//
// Generic type promotion inference: core scalar types (8-32 bit)
// Tests 22 type pair combinations.
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

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int idx = 0;

    // Case 0: int8_t + int16_t -> T = int16_t (code -2)
    {
        int8_t a = int8_t(3);
        int16_t b = int16_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -2
    }

    // Case 1: int8_t + int32_t -> T = int32_t (code -4)
    {
        int8_t a = int8_t(3);
        int32_t b = 7;
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -4
    }

    // Case 2: int8_t + uint8_t -> T = uint8_t (code 1)
    {
        int8_t a = int8_t(3);
        uint8_t b = uint8_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 1
    }

    // Case 3: int8_t + uint16_t -> T = uint16_t (code 2)
    {
        int8_t a = int8_t(3);
        uint16_t b = uint16_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 2
    }

    // Case 4: int8_t + uint32_t -> T = uint32_t (code 4)
    {
        int8_t a = int8_t(3);
        uint32_t b = 7u;
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 4
    }

    // Case 5: int8_t + float -> T = float (code -4)
    {
        int8_t a = int8_t(3);
        float b = float(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -4
    }

    // Case 6: int16_t + int32_t -> T = int32_t (code -4)
    {
        int16_t a = int16_t(3);
        int32_t b = 7;
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -4
    }

    // Case 7: int16_t + uint8_t -> T = int16_t (code -2)
    {
        int16_t a = int16_t(3);
        uint8_t b = uint8_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -2
    }

    // Case 8: int16_t + uint16_t -> T = uint16_t (code 2)
    {
        int16_t a = int16_t(3);
        uint16_t b = uint16_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 2
    }

    // Case 9: int16_t + uint32_t -> T = uint32_t (code 4)
    {
        int16_t a = int16_t(3);
        uint32_t b = 7u;
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 4
    }

    // Case 10: int16_t + float -> T = float (code -4)
    {
        int16_t a = int16_t(3);
        float b = float(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -4
    }

    // Case 11: int32_t + uint8_t -> T = int32_t (code -4)
    {
        int32_t a = 3;
        uint8_t b = uint8_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -4
    }

    // Case 12: int32_t + uint16_t -> T = int32_t (code -4)
    {
        int32_t a = 3;
        uint16_t b = uint16_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -4
    }

    // Case 13: int32_t + uint32_t -> T = uint32_t (code 4)
    {
        int32_t a = 3;
        uint32_t b = 7u;
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 4
    }

    // Case 14: int32_t + float -> T = float (code -4)
    {
        int32_t a = 3;
        float b = float(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -4
    }

    // Case 15: uint8_t + uint16_t -> T = uint16_t (code 2)
    {
        uint8_t a = uint8_t(3);
        uint16_t b = uint16_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 2
    }

    // Case 16: uint8_t + uint32_t -> T = uint32_t (code 4)
    {
        uint8_t a = uint8_t(3);
        uint32_t b = 7u;
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 4
    }

    // Case 17: uint8_t + float -> T = float (code -4)
    {
        uint8_t a = uint8_t(3);
        float b = float(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -4
    }

    // Case 18: uint16_t + uint32_t -> T = uint32_t (code 4)
    {
        uint16_t a = uint16_t(3);
        uint32_t b = 7u;
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 4
    }

    // Case 19: uint16_t + float -> T = float (code -4)
    {
        uint16_t a = uint16_t(3);
        float b = float(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -4
    }

    // Case 20: uint32_t + float -> T = float (code -4)
    {
        uint32_t a = 3u;
        float b = float(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -4
    }

    // Case 21: half + float -> T = float (code -4)
    {
        half a = half(3);
        float b = float(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -4
    }
}
