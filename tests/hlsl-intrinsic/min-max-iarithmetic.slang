//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-wgpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-mtl -compute -shaderobj

// Test that min/max can be used with IArithmetic constraint (issue #9395)
// This verifies that generic code can use min/max for both integer and float types.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Custom type conforming to IArithmetic to test the lessThan fallback path
struct MyNumber : IArithmetic
{
    int value;

    // IComparable requirements
    bool equals(This other) { return value == other.value; }
    bool lessThan(This other) { return value < other.value; }
    bool lessThanOrEquals(This other) { return value <= other.value; }

    // IArithmetic requirements
    This add(This other) { MyNumber r; r.value = value + other.value; return r; }
    This sub(This other) { MyNumber r; r.value = value - other.value; return r; }
    This mul(This other) { MyNumber r; r.value = value * other.value; return r; }
    This div(This other) { MyNumber r; r.value = value / other.value; return r; }
    This mod(This other) { MyNumber r; r.value = value % other.value; return r; }
    This neg() { MyNumber r; r.value = -value; return r; }

    __init(int val) { value = val; }
    __init(This other) { value = other.value; }
}

// Generic min function using IArithmetic constraint (works for both int and float)
T genericMinArithmetic<T : IArithmetic>(T a, T b)
{
    return min(a, b);
}

// Generic max function using IArithmetic constraint (works for both int and float)
T genericMaxArithmetic<T : IArithmetic>(T a, T b)
{
    return max(a, b);
}

// Generic min function using IInteger constraint (integer-specific)
T genericMinInteger<T : IInteger>(T a, T b)
{
    return min(a, b);
}

// Generic max function using IFloat constraint (float-specific)
T genericMaxFloat<T : IFloat>(T a, T b)
{
    return max(a, b);
}

// Generic function that uses both min and max
T genericClamp<T : IArithmetic>(T val, T minVal, T maxVal)
{
    return min(max(val, minVal), maxVal);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test IArithmetic constraint with integers
    int intA = 5;
    int intB = 3;
    
    int intMin = genericMinArithmetic<int>(intA, intB);  // Expected: 3
    int intMax = genericMaxArithmetic<int>(intA, intB);  // Expected: 5
    int intClamped = genericClamp<int>(7, 2, 6);  // Expected: 6
    
    outputBuffer[0] = intMin;      // 3
    outputBuffer[1] = intMax;      // 5
    outputBuffer[2] = intClamped;  // 6
    
    // Test IArithmetic constraint with floats
    float floatA = 2.5f;
    float floatB = 4.0f;
    
    float floatMin = genericMinArithmetic<float>(floatA, floatB);  // Expected: 2.5
    float floatMax = genericMaxArithmetic<float>(floatA, floatB);  // Expected: 4.0
    float floatClamped = genericClamp<float>(5.0f, 1.0f, 3.0f);  // Expected: 3.0
    
    outputBuffer[3] = int(floatMin * 10);      // 25
    outputBuffer[4] = int(floatMax * 10);      // 40
    outputBuffer[5] = int(floatClamped * 10);  // 30
    
    // Test IInteger constraint
    int intMinI = genericMinInteger<int>(intA, intB);  // Expected: 3
    outputBuffer[6] = intMinI;  // 3
    
    // Test IFloat constraint
    float floatMaxF = genericMaxFloat<float>(floatA, floatB);  // Expected: 4.0
    outputBuffer[7] = int(floatMaxF * 10);  // 40
    
    // Test that direct calls still work (backward compatibility)
    outputBuffer[8] = min(10, 20);  // 10 (int)
    outputBuffer[9] = int(max(1.5f, 2.5f) * 10);  // 25 (float)
    
    // Test with uint (unsigned integer)
    uint uintA = 8u;
    uint uintB = 12u;
    outputBuffer[10] = int(genericMinArithmetic<uint>(uintA, uintB));  // 8
    outputBuffer[11] = int(genericMaxArithmetic<uint>(uintA, uintB));  // 12

    // Test with custom IArithmetic type (uses lessThan fallback)
    MyNumber numA = MyNumber(15);
    MyNumber numB = MyNumber(7);
    MyNumber customMin = genericMinArithmetic<MyNumber>(numA, numB);  // Expected: 7
    MyNumber customMax = genericMaxArithmetic<MyNumber>(numA, numB);  // Expected: 15
    outputBuffer[12] = customMin.value;  // 7
    outputBuffer[13] = customMax.value;  // 15
}

