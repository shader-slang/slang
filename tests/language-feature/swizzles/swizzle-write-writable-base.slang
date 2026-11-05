//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -shaderobj -output-using-type

// Matrix and tuple swizzles are l-values exactly when their base is. Each case below writes
// through a swizzle whose base is writable for a different reason: a field of `this` in a
// `[mutating]` method, a field of a `var` struct, an `inout` parameter, and a buffer element.

struct Holder
{
    float2x2 m;
    Tuple<float, float> t;

    [mutating]
    void set()
    {
        m._m01 = 1;
        t._1 = 2;
    }
}

void setInOut(inout float2x2 m, inout Tuple<float, float> t)
{
    m._m10_m11 = float2(5, 6);
    t._0 = 7;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=16):name=matrixBuffer
RWStructuredBuffer<float2x2> matrixBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Holder h = { float2x2(0), makeTuple(0.0, 0.0) };
    h.set();

    var g = Holder(float2x2(0), makeTuple(0.0, 0.0));
    g.m._11 = 3;
    g.t._0 = 4;

    var m = float2x2(0);
    var t = makeTuple(0.0, 0.0);
    setInOut(m, t);

    matrixBuffer[0]._m00_m11 = float2(8, 9);

    outputBuffer[0] = h.m[0][1];
    outputBuffer[1] = h.t._1;
    outputBuffer[2] = g.m[0][0];
    outputBuffer[3] = g.t._0;
    outputBuffer[4] = m[1][0] + m[1][1];
    outputBuffer[5] = t._0;
    outputBuffer[6] = matrixBuffer[0][0][0];
    outputBuffer[7] = matrixBuffer[0][1][1];
}

// CHECK: 1.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 3.0
// CHECK-NEXT: 4.0
// CHECK-NEXT: 11.0
// CHECK-NEXT: 7.0
// CHECK-NEXT: 8.0
// CHECK-NEXT: 9.0
