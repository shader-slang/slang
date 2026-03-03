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

extension<int N> Array<ITerm, N>
{
    float4 sumOver(float4 u)
    {
        float4 v = 0.f;
        for (var i = 0; i < N; i++)
            v += this[i].evaluate(u);
        return v;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID) {

    let pixel = uint2(threadId.x, threadId.y);
    Array<ITerm, 2> terms1 = { Multiply(4.f), Pow(3) };

    // 4 * 0.5 + (0.5 ^ 3) = 2 + 0.125 = 2.125
    outputBuffer[0] = terms1.sumOver(float4(0.5)).x; // CHECK: 2.125

    Array<ITerm, 3> terms2 = {Multiply(3.f), Multiply(4.f), Multiply(5.f) };

    // 3 * 0.5 + 4 * 0.5 + 5 * 0.5 = 1.5 + 2 + 2.5 = 6
    outputBuffer[1] = terms2.sumOver(float4(0.5)).x; // CHECK: 6
}