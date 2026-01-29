//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -Xslang -Wno-41020

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//
// Test vectors of interface types
// 

public interface ITerm 
{
    float4 evaluate(float4 u);
}

public struct Multiply : ITerm
{
    float factor;
    public __init(float factor) { this.factor = factor; }
    public float4 evaluate(float4 u) { return u * factor; }
}

public struct Pow : ITerm
{
    int exponent;
    public __init(int exponent) { this.exponent = exponent; }
    public float4 evaluate(float4 u) { return pow(u.x, exponent); }
}

float4 diff(Tuple<ITerm, ITerm> terms, float4 u)
{
    return terms._0.evaluate(u) - terms._1.evaluate(u);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID) {

    let pixel = uint2(threadId.x, threadId.y);
    Tuple<ITerm, ITerm> terms1 = { Multiply(4.f), Pow(3) };

    // 4 * 0.5 - (0.5 ^ 3) = 2 - 0.125 = 1.875
    outputBuffer[0] = diff(terms1, float4(0.5)).x; // CHECK: 1.875

    Tuple<ITerm, ITerm> terms2 = { Multiply(3.f), Multiply(4.f) };

    // 3 * 0.5 - 4 * 0.5 = 1.5 - 2 = -0.5
    outputBuffer[1] = diff(terms2, float4(0.5)).x; // CHECK: -0.5
}