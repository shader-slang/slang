// THIS IS A GENERATED FILE. DO NOT EDIT!
// Instead, edit extras/test-generators/generate-generic-type-promotion-tests.py
// and regenerate by running:
//
//     python3 extras/test-generators/generate-generic-type-promotion-tests.py
//
// Generic type promotion inference: 64-bit scalar types
// Tests 44 type pair combinations.
//
// Each case validates both the arithmetic result (add) and the inferred type
// identity (getInferredTypeCode). The type code encodes sizeof(T) with sign:
//   positive = unsigned type, negative = signed or float type.
//
// Bool pairs always join: against integer types the integer side is
// strictly better, and against floating-point types the one-off
// bool-loses tie-break applies, matching TryJoinTypes. Bool itself
// therefore never appears as an inferred type here.
//
// Excluded pairs: integer+half (E30081, CPU backend limitation),
// float+double (E30082 performance warning), and bool+floating or
// bool+pointer-sized-signed (E30081 for the `bool` operand).
//
// 64-bit types (int64_t, uint64_t, double, intptr_t, uintptr_t) may
// require extensions on GPU backends, so this test runs on CPU only.

//TEST:COMPARE_COMPUTE(targets=cuda+cpu, filecheck-buffer=CHECK): -output-using-type

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

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int idx = 0;

    // Case 0: bool + int64_t -> T = int64_t (code -8)
    {
        bool a = true;
        int64_t b = int64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 8
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 1: bool + uint64_t -> T = uint64_t (code 8)
    {
        bool a = true;
        uint64_t b = uint64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 8
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 2: bool + uintptr_t -> T = uintptr_t (code 8)
    {
        bool a = true;
        uintptr_t b = uintptr_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 8
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 3: int8_t + int64_t -> T = int64_t (code -8)
    {
        int8_t a = int8_t(3);
        int64_t b = int64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 4: int8_t + uint64_t -> T = uint64_t (code 8)
    {
        int8_t a = int8_t(3);
        uint64_t b = uint64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 5: int8_t + intptr_t -> T = intptr_t (code -8)
    {
        int8_t a = int8_t(3);
        intptr_t b = intptr_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 6: int8_t + uintptr_t -> T = uintptr_t (code 8)
    {
        int8_t a = int8_t(3);
        uintptr_t b = uintptr_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 7: int8_t + double -> T = double (code -8)
    {
        int8_t a = int8_t(3);
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 8: int16_t + int64_t -> T = int64_t (code -8)
    {
        int16_t a = int16_t(3);
        int64_t b = int64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 9: int16_t + uint64_t -> T = uint64_t (code 8)
    {
        int16_t a = int16_t(3);
        uint64_t b = uint64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 10: int16_t + intptr_t -> T = intptr_t (code -8)
    {
        int16_t a = int16_t(3);
        intptr_t b = intptr_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 11: int16_t + uintptr_t -> T = uintptr_t (code 8)
    {
        int16_t a = int16_t(3);
        uintptr_t b = uintptr_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 12: int16_t + double -> T = double (code -8)
    {
        int16_t a = int16_t(3);
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 13: int32_t + int64_t -> T = int64_t (code -8)
    {
        int32_t a = 3;
        int64_t b = int64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 14: int32_t + uint64_t -> T = uint64_t (code 8)
    {
        int32_t a = 3;
        uint64_t b = uint64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 15: int32_t + intptr_t -> T = intptr_t (code -8)
    {
        int32_t a = 3;
        intptr_t b = intptr_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 16: int32_t + uintptr_t -> T = uintptr_t (code 8)
    {
        int32_t a = 3;
        uintptr_t b = uintptr_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 17: int32_t + double -> T = double (code -8)
    {
        int32_t a = 3;
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 18: uint8_t + int64_t -> T = int64_t (code -8)
    {
        uint8_t a = uint8_t(3);
        int64_t b = int64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 19: uint8_t + uint64_t -> T = uint64_t (code 8)
    {
        uint8_t a = uint8_t(3);
        uint64_t b = uint64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 20: uint8_t + uintptr_t -> T = uintptr_t (code 8)
    {
        uint8_t a = uint8_t(3);
        uintptr_t b = uintptr_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 21: uint8_t + double -> T = double (code -8)
    {
        uint8_t a = uint8_t(3);
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 22: uint16_t + int64_t -> T = int64_t (code -8)
    {
        uint16_t a = uint16_t(3);
        int64_t b = int64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 23: uint16_t + uint64_t -> T = uint64_t (code 8)
    {
        uint16_t a = uint16_t(3);
        uint64_t b = uint64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 24: uint16_t + uintptr_t -> T = uintptr_t (code 8)
    {
        uint16_t a = uint16_t(3);
        uintptr_t b = uintptr_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 25: uint16_t + double -> T = double (code -8)
    {
        uint16_t a = uint16_t(3);
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 26: uint32_t + int64_t -> T = int64_t (code -8)
    {
        uint32_t a = 3u;
        int64_t b = int64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 27: uint32_t + uint64_t -> T = uint64_t (code 8)
    {
        uint32_t a = 3u;
        uint64_t b = uint64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 28: uint32_t + uintptr_t -> T = uintptr_t (code 8)
    {
        uint32_t a = 3u;
        uintptr_t b = uintptr_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 29: uint32_t + double -> T = double (code -8)
    {
        uint32_t a = 3u;
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 30: half + double -> T = double (code -8)
    {
        half a = half(3);
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 31: float + int64_t -> T = float (code -4)
    {
        float a = float(3);
        int64_t b = int64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -4
    }

    // Case 32: float + uint64_t -> T = float (code -4)
    {
        float a = float(3);
        uint64_t b = uint64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -4
    }

    // Case 33: float + intptr_t -> T = float (code -4)
    {
        float a = float(3);
        intptr_t b = intptr_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -4
    }

    // Case 34: float + uintptr_t -> T = float (code -4)
    {
        float a = float(3);
        uintptr_t b = uintptr_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -4
    }

    // Case 35: int64_t + uint64_t -> T = uint64_t (code 8)
    {
        int64_t a = int64_t(3);
        uint64_t b = uint64_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 36: int64_t + intptr_t -> T = int64_t (code -8)
    {
        int64_t a = int64_t(3);
        intptr_t b = intptr_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 37: int64_t + double -> T = double (code -8)
    {
        int64_t a = int64_t(3);
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 38: uint64_t + intptr_t -> T = uint64_t (code 8)
    {
        uint64_t a = uint64_t(3);
        intptr_t b = intptr_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 39: uint64_t + uintptr_t -> T = uint64_t (code 8)
    {
        uint64_t a = uint64_t(3);
        uintptr_t b = uintptr_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 40: uint64_t + double -> T = double (code -8)
    {
        uint64_t a = uint64_t(3);
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 41: intptr_t + uintptr_t -> T = uintptr_t (code 8)
    {
        intptr_t a = intptr_t(3);
        uintptr_t b = uintptr_t(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: 8
    }

    // Case 42: intptr_t + double -> T = double (code -8)
    {
        intptr_t a = intptr_t(3);
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }

    // Case 43: uintptr_t + double -> T = double (code -8)
    {
        uintptr_t a = uintptr_t(3);
        double b = double(7);
        outputBuffer[idx++] = int(add(a, b));
        // CHECK: 10
        outputBuffer[idx++] = getInferredTypeCode(a, b);
        // CHECK: -8
    }
}
