//TEST:SIMPLE(filecheck=CHK):-target hlsl -stage compute -entry computeMain

// Test global static_assert support with improved test coverage based on feedback

struct TestStruct {
    float a;
    int b;
};

// Global static_assert should work now
test_static_assert(sizeof(float) == 4, "float should be 4 bytes");
test_static_assert(sizeof(int) == 4, "int should be 4 bytes");  
test_static_assert(sizeof(TestStruct) == 8, "TestStruct should be 8 bytes");

// Test with layout attributes - std430 layout with padding behavior
[layout(std430)]
struct LayoutStruct {
    float3 x;  // 12 bytes
    float2 y;  // 8 bytes, but aligned to 16-byte boundary due to float3
};

// The float3 causes the float2 to be aligned, resulting in sizeof(float4) + sizeof(float2) = 24 bytes
test_static_assert(sizeof(LayoutStruct) == 24, "LayoutStruct should be 24 bytes with padding");

// Test with std140 layout containing array of float3
[layout(std140)]
struct Std140Struct {
    float3 data[2];  // In std140, each float3 is aligned to 16 bytes like float4
};

// In std140, float3 array elements are treated as float4, so 2 * 16 = 32 bytes
test_static_assert(sizeof(Std140Struct) == 32, "Std140Struct should be 32 bytes");

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Function-scope static_assert should still work
    static_assert(sizeof(float2) == 8, "float2 should be 8 bytes");
    static_assert(sizeof(matrix<float, 2, 2>) == 16, "2x2 matrix should be 16 bytes");
    // Test float3x4 matrix size (3 columns of float4)
    static_assert(sizeof(float3x4) == 48, "float3x4 should be 48 bytes");
}

// CHK-NOT: error