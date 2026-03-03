//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -output-using-type -emit-spirv-directly
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -dx12 -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -cuda -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF): -mtl -output-using-type
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4, count=10):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Test __floatAsInt with different floating-point types and literals

// Float literals (suffix f)
static const int floatOne = __floatAsInt(1.0f);          // 0x3F800000 = 1065353216
static const int floatNegOne = __floatAsInt(-1.0f);      // 0xBF800000 = -1082130432
static const int floatZero = __floatAsInt(0.0f);         // 0x00000000 = 0

// Float with explicit cast
static const int floatFromInt = __floatAsInt(float(1));  // 0x3F800000 = 1065353216

// Test using the constants in array size (proves compile-time evaluation)
static const int arraySize = (__floatAsInt(2.0f) >> 23) - 126; // extracts exponent, should be 2
int testArray[arraySize];

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // BUF: 1065353216
    outputBuffer[0] = floatOne;

    // BUF-NEXT: -1082130432
    outputBuffer[1] = floatNegOne;

    // BUF-NEXT: 0
    outputBuffer[2] = floatZero;

    // BUF-NEXT: 1065353216
    outputBuffer[3] = floatFromInt;

    // Direct usage
    // BUF-NEXT: 1065353216
    outputBuffer[4] = __floatAsInt(1.0f);

    // BUF-NEXT: 1073741824
    outputBuffer[5] = __floatAsInt(2.0f);  // 0x40000000

    // BUF-NEXT: 1082130432
    outputBuffer[6] = __floatAsInt(4.0f);  // 0x40800000

    // Test array size is correct (should be 2)
    // BUF-NEXT: 2
    outputBuffer[7] = arraySize;

    // Pi approximation: 3.14159f = 0x40490FD0 = 1078530000
    // BUF-NEXT: 1078530000
    outputBuffer[8] = __floatAsInt(3.14159f);

    // Negative zero: -0.0f = 0x80000000 = -2147483648
    // BUF-NEXT: -2147483648
    outputBuffer[9] = __floatAsInt(-0.0f);
}
