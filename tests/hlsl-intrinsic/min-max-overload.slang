//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -O0

// Test that min/max with IInteger and IFloat constraints emit optimized intrinsics (issue #9395)
// This verifies that the IArithmetic overload routes to the correct SPIR-V instructions.

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

// Generic min function using IInteger constraint
T genericMin<T : IInteger>(T a, T b)
{
    return min(a, b);
}

// Generic max function using IFloat constraint
T genericMax<T : IFloat>(T a, T b)
{
    return max(a, b);
}

// Generic min/max functions using IArithmetic constraint (works for custom types)
T genericMinArithmetic<T : IArithmetic>(T a, T b)
{
    return min(a, b);
}

T genericMaxArithmetic<T : IArithmetic>(T a, T b)
{
    return max(a, b);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Test IInteger constraint - should emit SMin
    int intA = 5;
    int intB = 3;
    int intMin = genericMin<int>(intA, intB);
    // CHECK: SMin

    // Test IFloat constraint - should emit FMax
    float floatA = 2.5f;
    float floatB = 4.0f;
    float floatMax = genericMax<float>(floatA, floatB);
    // CHECK: FMax

    outputBuffer[0] = intMin;
    outputBuffer[1] = int(floatMax * 10);

    // Test direct calls - should also emit optimized intrinsics
    outputBuffer[2] = min(10, 20);
    // CHECK: SMin

    outputBuffer[3] = int(max(1.5f, 2.5f) * 10);
    // CHECK: FMax

    // Test custom IArithmetic type - should use lessThan comparison + Select
    MyNumber numA = MyNumber(15);
    MyNumber numB = MyNumber(7);
    MyNumber customMin = genericMinArithmetic<MyNumber>(numA, numB);
    MyNumber customMax = genericMaxArithmetic<MyNumber>(numA, numB);
    // CHECK: OpSLessThan
    // CHECK: OpSelectionMerge

    outputBuffer[4] = customMin.value;  // 7
    outputBuffer[5] = customMax.value;  // 15
}
