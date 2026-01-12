//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -O0

// Test that min/max with IInteger and IFloat constraints emit optimized intrinsics (issue #9395)
// This verifies that the IArithmetic overload routes to the correct SPIR-V instructions.

RWStructuredBuffer<int> outputBuffer;

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
}
