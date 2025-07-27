//TEST:SIMPLE(filecheck=CHK):-target hlsl -stage compute -entry computeMain

// Test global static_assert support 
struct TestStruct {
    float a;
    int b;
};

// Global static_assert should work now
static_assert(sizeof(float) == 4, "float should be 4 bytes");
static_assert(sizeof(int) == 4, "int should be 4 bytes");  
static_assert(sizeof(TestStruct) == 8, "TestStruct should be 8 bytes");

// Test with layout attributes
[layout(std430)]
struct LayoutStruct {
    float x;
    float y;
};

static_assert(sizeof(LayoutStruct) == 8, "LayoutStruct should be 8 bytes");

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Function-scope static_assert should still work
    static_assert(sizeof(float2) == 8, "float2 should be 8 bytes");
    static_assert(sizeof(matrix<float, 2, 2>) == 16, "2x2 matrix should be 16 bytes");
}

// CHK-NOT: error