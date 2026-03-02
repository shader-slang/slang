// Auto-inference of AnyValue size when implementations have significantly
// different sizes. The compiler computes the maximum size needed across all
// conforming types when no explicit [anyValueSize] attribute is provided.
//
// Unlike simple.slang (empty structs with zero-byte data fields), this test
// uses types with a 4:1 size ratio (4 bytes vs 16 bytes) to exercise the
// "pick the maximum" inference logic.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// No [anyValueSize] -- the compiler infers from the largest conforming type.
interface IShape
{
    float area();
}

// Small implementation: 1 float field = 4 bytes
struct Circle : IShape
{
    float radius;
    float area() { return 3.14159f * radius * radius; }
}

// Large implementation: 4 float fields = 16 bytes.
// The inferred AnyValue size must be at least 16 to fit this type.
struct Rectangle : IShape
{
    float x;
    float y;
    float width;
    float height;
    float area() { return width * height; }
}

float computeArea(uint id)
{
    IShape s;
    if (id == 0)
    {
        Circle c;
        c.radius = 1.0;
        s = c;
    }
    else
    {
        Rectangle r;
        r.x = 0.0;
        r.y = 0.0;
        r.width = 3.0;
        r.height = 4.0;
        s = r;
    }

    return s.area();
    // REPORT: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Circle with radius=1: area = pi * 1 * 1 = 3.14159
    outputBuffer[0] = computeArea(0);
    // CHECK: 3.14159

    // Rectangle 3x4: area = 12.0
    outputBuffer[1] = computeArea(1);
    // CHECK: 12.0
}
