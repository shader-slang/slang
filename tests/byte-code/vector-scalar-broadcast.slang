//TEST:INTERPRET(filecheck=CHECK):

float3 divide(float3 value, float divisor)
{
    return value / divisor;
}

int4 subtract(int4 value, int amount)
{
    return value - amount;
}

int4 reverseSubtract(int amount, int4 value)
{
    return amount - value;
}

uint16_t4 shiftLeft(uint16_t4 value, int amount)
{
    return value << amount;
}

bool4 greaterThan(int4 value, int threshold)
{
    return value > threshold;
}

float2x2 scale(float2x2 value, float amount)
{
    return value * amount;
}

export __extern_cpp int main()
{
    float3 quotient = divide(float3(126.0, 128.0, 128.0), 255.0);
    //CHECK: quotient: 0.494118 0.501961 0.501961
    printf("quotient: %.6f %.6f %.6f\n", quotient.x, quotient.y, quotient.z);

    int4 difference = subtract(int4(1, 2, 3, 4), 1);
    //CHECK: difference: 0 1 2 3
    printf(
        "difference: %d %d %d %d\n",
        difference.x,
        difference.y,
        difference.z,
        difference.w);

    int4 reverseDifference = reverseSubtract(4, int4(1, 2, 3, 4));
    //CHECK: reverse: 3 2 1 0
    printf(
        "reverse: %d %d %d %d\n",
        reverseDifference.x,
        reverseDifference.y,
        reverseDifference.z,
        reverseDifference.w);

    uint16_t4 shifted = shiftLeft(uint16_t4(1, 2, 3, 4), 1);
    //CHECK: shifted: 2 4 6 8
    printf(
        "shifted: %d %d %d %d\n",
        int(shifted.x),
        int(shifted.y),
        int(shifted.z),
        int(shifted.w));

    bool4 comparison = greaterThan(int4(1, 2, 3, 4), 2);
    //CHECK: comparison: 0 0 1 1
    printf(
        "comparison: %d %d %d %d\n",
        int(comparison.x),
        int(comparison.y),
        int(comparison.z),
        int(comparison.w));

    float2x2 scaled = scale(float2x2(1.0, 2.0, 3.0, 4.0), 2.0);
    //CHECK: scaled: 2.0 4.0 6.0 8.0
    printf(
        "scaled: %.1f %.1f %.1f %.1f\n",
        scaled[0][0],
        scaled[0][1],
        scaled[1][0],
        scaled[1][1]);
    return 0;
}
