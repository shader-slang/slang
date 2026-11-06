// gh-14259-hlsl-typed-special-literals-fxc.slang

// A `double` or `half` infinity used in arithmetic is written as a cast of a division in its own
// literals, `double(1.0L / 0.0L)` or `half(-1.0 / 0.0)`, on every HLSL profile, including the
// SM 5.0 profile that FXC compiles. The dx11 lane compiles these spellings with FXC.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute -profile cs_5_0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-slang -compute -dx11 -shaderobj -render-feature double
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // The scale depends on the thread ID, so each product is computed by the shader rather than
    // folded by Slang.
    double scale = double(tid.x + 1);

    // HLSL-DAG: double(1.0L / 0.0L) *
    double positiveInfinity = double(1.0 / 0.0) * scale;
    // HLSL-DAG: half(-1.0 / 0.0) *
    half negativeInfinity = half(-1.0 / 0.0) * half(scale);

    uint low;
    uint high;
    asuint(positiveInfinity, low, high);
    outputBuffer[0] = low;
    outputBuffer[1] = high;
    outputBuffer[2] = asuint(float(negativeInfinity));

    // BUF: 0
    // BUF-NEXT: 7FF00000
    // BUF-NEXT: FF800000
}
