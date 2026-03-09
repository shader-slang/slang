// Auto-inference of AnyValue size with many implementations of varying sizes.
// The compiler must select the maximum size across ALL conforming types, even
// those not used at a particular dispatch site.
//
// Four implementations with sizes: 4, 8, 16, and 32 bytes. The inferred
// AnyValue size must be at least 32 to accommodate the largest.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites
//TEST:SIMPLE(filecheck=SIZE):-target spirv-asm -stage compute -entry computeMain -dump-ir-after inferAnyValueSizeWhereNecessary

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// No [anyValueSize] -- compiler infers from the largest conforming type.
interface IData
{
    int sum();
}

// 4 bytes: 1 int
struct Tiny : IData
{
    int a;
    int sum() { return a; }
}

// 8 bytes: 2 ints
struct Small : IData
{
    int a;
    int b;
    int sum() { return a + b; }
}

// 16 bytes: 4 ints
struct Medium : IData
{
    int a;
    int b;
    int c;
    int d;
    int sum() { return a + b + c + d; }
}

// 32 bytes: 8 ints -- this is the largest, drives the inferred size
struct Large : IData
{
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
    int g;
    int h;
    int sum() { return a + b + c + d + e + f + g + h; }
}

IData createIData(uint id)
{
    IData d;
    if (id == 0)
    {
        Tiny t;
        t.a = 1;
        d = t;
    }
    else if (id == 1)
    {
        Small s;
        s.a = 2;
        s.b = 3;
        d = s;
    }
    else if (id == 2)
    {
        Medium m;
        m.a = 1;
        m.b = 2;
        m.c = 3;
        m.d = 4;
        d = m;
    }
    else
    {
        Large l;
        l.a = 1;
        l.b = 2;
        l.c = 3;
        l.d = 4;
        l.e = 5;
        l.f = 6;
        l.g = 7;
        l.h = 8;
        d = l;
    }
    return d;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    IData d = createIData(dispatchThreadID.x);
    outputBuffer[dispatchThreadID.x] = d.sum();
    // REPORT: {{.*}} generated {{.*}} dispatch code{{.*}} 4 possible types:
}
// Verify inferred AnyValueSize and struct sizes via IR dump.
// SIZE: AnyValueSize(32 : Int)
// SIZE: SizeAndAlignment({{.*}}, 4 : Int64, {{.*}})
// SIZE: struct %Tiny
// SIZE: SizeAndAlignment({{.*}}, 8 : Int64, {{.*}})
// SIZE: struct %Small
// SIZE: SizeAndAlignment({{.*}}, 16 : Int64, {{.*}})
// SIZE: struct %Medium
// SIZE: SizeAndAlignment({{.*}}, 32 : Int64, {{.*}})
// SIZE: struct %Large
// Tiny: 1
// Small: 2+3 = 5
// Medium: 1+2+3+4 = 10
// Large: 1+2+3+4+5+6+7+8 = 36
// CHECK: 1
// CHECK: 5
// CHECK: 10
// CHECK: 36
