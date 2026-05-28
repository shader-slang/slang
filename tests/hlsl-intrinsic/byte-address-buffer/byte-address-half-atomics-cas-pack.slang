//TEST:INTERPRET(filecheck=CHECK):

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

void main()
{
    half originalValue;
    uint packed0 = simulateRetry(0x40003C00U, 0x42003C00U, 0U, 2.0h, originalValue);
    printf("lane0 %.1f %u\n", float(originalValue), packed0);

    uint packed1 = simulateRetry(0x40003C00U, 0x40004200U, 16U, 1.0h, originalValue);
    printf("lane1 %.1f %u\n", float(originalValue), packed1);
}

// CHECK: lane0 1.0 1107313152
// CHECK: lane1 2.0 1107313152
