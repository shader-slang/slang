// gh-14259-hlsl-double-literal-suffix-fxc.slang

// A `double` literal in arithmetic keeps all 64 bits on every HLSL profile, including the SM 5.0
// profile that FXC compiles. FXC reads an unsuffixed floating-point literal as a 32-bit `float`,
// so the emitted literal carries the `L` suffix. With `0.1` read as a `float`, the low word of
// the product would be `A0000000` instead of `9999999A`.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute -profile cs_5_0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-slang -compute -dx11 -shaderobj -render-feature double
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-slang -compute -dx12 -shaderobj -render-feature double
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // The operand depends on the thread ID, so the product is computed by the shader rather than
    // folded by Slang.
    double d = double(tid.x + 2);
    // HLSL: {{.*}} * 0.10000000000000001L
    double product = d * 0.1l;

    uint low;
    uint high;
    asuint(product, low, high);
    outputBuffer[0] = low;
    outputBuffer[1] = high;

    // BUF: 9999999A
    // BUF-NEXT: 3FC99999
}
