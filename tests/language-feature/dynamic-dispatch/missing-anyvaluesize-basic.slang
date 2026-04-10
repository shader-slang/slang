// Auto-inference of AnyValue size when implementations have significantly
// different sizes. The compiler computes the maximum size needed across all
// conforming types when no explicit [anyValueSize] attribute is provided.
//
// Unlike simple.slang (empty structs with zero-byte data fields), this test
// uses types with a 4:1 size ratio (4 bytes vs 16 bytes) to exercise the
// "pick the maximum" inference logic.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites
//TEST:SIMPLE(filecheck=SIZE):-target spirv-asm -stage compute -entry computeMain -dump-ir-after inferAnyValueSizeWhereNecessary

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// No [anyValueSize] -- the compiler infers from the largest conforming type.
interface IShape
{
    int compute();
}

// Small implementation: 1 int field = 4 bytes
struct Circle : IShape
{
    int radius;
    int compute() { return radius * radius; }
}

// Large implementation: 4 int fields = 16 bytes.
// The inferred AnyValue size must be at least 16 to fit this type.
struct Rectangle : IShape
{
    int x;
    int y;
    int width;
    int height;
    int compute() { return width * height; }
}

IShape createShape(uint id)
{
    IShape s;
    if (id == 0)
    {
        Circle c;
        c.radius = 5;
        s = c;
    }
    else
    {
        Rectangle r;
        r.x = 0;
        r.y = 0;
        r.width = 3;
        r.height = 4;
        s = r;
    }
    return s;
}

[numthreads(2, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    IShape s = createShape(dispatchThreadID.x);
    outputBuffer[dispatchThreadID.x] = s.compute();
    // REPORT: {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}
// Verify inferred AnyValueSize and struct sizes via IR dump.
// SIZE: AnyValueSize(16 : Int)
// SIZE: SizeAndAlignment({{.*}}, 4 : Int64, {{.*}})
// SIZE: struct %Circle
// SIZE: SizeAndAlignment({{.*}}, 16 : Int64, {{.*}})
// SIZE: struct %Rectangle
// Circle(radius=5): 5*5 = 25
// Rectangle(3x4): 3*4 = 12
// CHECK: 25
// CHECK: 12
