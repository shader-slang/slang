//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

uint simulateRetry(
    uint initialPacked,
    uint failedActualPacked,
    uint laneShift,
    half value,
    out half originalValue)
{
    uint oldPacked = initialPacked;

    half oldHalf;
    uint newPacked = __packInterlockedAddF16Result(oldPacked, laneShift, value, oldHalf);
    if (__commitInterlockedAddF16Result(failedActualPacked, oldPacked, oldHalf, originalValue))
    {
        return newPacked;
    }

    newPacked = __packInterlockedAddF16Result(oldPacked, laneShift, value, oldHalf);
    uint successActualPacked = oldPacked;
    if (__commitInterlockedAddF16Result(successActualPacked, oldPacked, oldHalf, originalValue))
    {
        return newPacked;
    }

    return 0U;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    half originalValue;
    uint packed0 = simulateRetry(0x40003C00U, 0x42003C00U, 0U, 2.0h, originalValue);
    outputBuffer[0] = uint(asuint16(originalValue));
    outputBuffer[1] = packed0;

    uint packed1 = simulateRetry(0x40003C00U, 0x40004200U, 16U, 1.0h, originalValue);
    outputBuffer[2] = uint(asuint16(originalValue));
    outputBuffer[3] = packed1;
}

// CHECK: 15360
// CHECK: 1107313152
// CHECK: 16384
// CHECK: 1107313152
