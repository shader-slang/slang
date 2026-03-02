// Auto-inference of AnyValue size with many implementations of varying sizes.
// The compiler must select the maximum size across ALL conforming types, even
// those not used at a particular dispatch site.
//
// Four implementations with sizes: 4, 8, 16, and 32 bytes. The inferred
// AnyValue size must be at least 32 to accommodate the largest.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// No [anyValueSize] -- compiler infers from the largest conforming type.
interface IData
{
    float sum();
}

// 4 bytes: 1 float
struct Tiny : IData
{
    float a;
    float sum() { return a; }
}

// 8 bytes: 2 floats
struct Small : IData
{
    float a;
    float b;
    float sum() { return a + b; }
}

// 16 bytes: 4 floats
struct Medium : IData
{
    float a;
    float b;
    float c;
    float d;
    float sum() { return a + b + c + d; }
}

// 32 bytes: 8 floats -- this is the largest, drives the inferred size
struct Large : IData
{
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;
    float g;
    float h;
    float sum() { return a + b + c + d + e + f + g + h; }
}

float computeSum(uint id)
{
    IData d;
    if (id == 0)
    {
        Tiny t;
        t.a = 1.0;
        d = t;
    }
    else if (id == 1)
    {
        Small s;
        s.a = 2.0;
        s.b = 3.0;
        d = s;
    }
    else if (id == 2)
    {
        Medium m;
        m.a = 1.0;
        m.b = 2.0;
        m.c = 3.0;
        m.d = 4.0;
        d = m;
    }
    else
    {
        Large l;
        l.a = 1.0;
        l.b = 2.0;
        l.c = 3.0;
        l.d = 4.0;
        l.e = 5.0;
        l.f = 6.0;
        l.g = 7.0;
        l.h = 8.0;
        d = l;
    }

    return d.sum();
    // REPORT: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 4 possible types:
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Tiny: sum = 1.0
    outputBuffer[0] = computeSum(0);
    // CHECK: 1.0

    // Small: sum = 2.0 + 3.0 = 5.0
    outputBuffer[1] = computeSum(1);
    // CHECK: 5.0

    // Medium: sum = 1+2+3+4 = 10.0
    outputBuffer[2] = computeSum(2);
    // CHECK: 10.0

    // Large: sum = 1+2+3+4+5+6+7+8 = 36.0
    outputBuffer[3] = computeSum(3);
    // CHECK: 36.0
}
