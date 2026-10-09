//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-llvm -output-using-type

// Regression test for shader-slang/slang#11075: generic `IFloat`/`IInteger` `min`/`max` on builtin
// scalars dispatch through `__IMinMax` to the concrete scalar overloads, so they return the same
// results as a direct call on each target, including for a NaN operand, whose handling differs
// between targets. Operands are loaded from a buffer so that the calls are not folded at compile
// time.

T minFloat<T : IFloat>(T a, T b)
{
    return min(a, b);
}
T maxFloat<T : IFloat>(T a, T b)
{
    return max(a, b);
}
T minInteger<T : IInteger>(T a, T b)
{
    return min(a, b);
}
T maxInteger<T : IInteger>(T a, T b)
{
    return max(a, b);
}

// 0x7FC00000 is a quiet NaN.
//TEST_INPUT:ubuffer(data=[0x7FC00000 0x3F800000 0x40000000], stride=4):name=inputBuffer
StructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float nan = asfloat(inputBuffer[0]);
    float one = asfloat(inputBuffer[1]);
    float two = asfloat(inputBuffer[2]);
    int three = int(two) + 1;

    int i = 0;
    outputBuffer[i++] = minFloat(one, two);                      // CHECK: 1.0
    outputBuffer[i++] = maxFloat(one, two);                      // CHECK-NEXT: 2.0
    outputBuffer[i++] = float(minFloat(double(two), double(one))); // CHECK-NEXT: 1.0
    outputBuffer[i++] = float(maxFloat(double(one), double(two))); // CHECK-NEXT: 2.0
    outputBuffer[i++] = float(minInteger(three, 9));             // CHECK-NEXT: 3.0
    outputBuffer[i++] = float(maxInteger(three, 9));             // CHECK-NEXT: 9.0
    outputBuffer[i++] = float(maxInteger(uint(three), 2u));      // CHECK-NEXT: 3.0
    outputBuffer[i++] = float(minInteger(int64_t(three), int64_t(-4))); // CHECK-NEXT: -4.000000

    outputBuffer[i++] = maxFloat(one, nan); // CHECK-NEXT: [[MAX_ONE_NAN:.+]]
    outputBuffer[i++] = max(one, nan);      // CHECK-NEXT: [[MAX_ONE_NAN]]
    outputBuffer[i++] = maxFloat(nan, one); // CHECK-NEXT: [[MAX_NAN_ONE:.+]]
    outputBuffer[i++] = max(nan, one);      // CHECK-NEXT: [[MAX_NAN_ONE]]
    outputBuffer[i++] = minFloat(two, nan); // CHECK-NEXT: [[MIN_TWO_NAN:.+]]
    outputBuffer[i++] = min(two, nan);      // CHECK-NEXT: [[MIN_TWO_NAN]]
    outputBuffer[i++] = minFloat(nan, two); // CHECK-NEXT: [[MIN_NAN_TWO:.+]]
    outputBuffer[i++] = min(nan, two);      // CHECK-NEXT: [[MIN_NAN_TWO]]
}
