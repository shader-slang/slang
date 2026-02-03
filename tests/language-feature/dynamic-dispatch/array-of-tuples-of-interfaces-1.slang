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

extension<int N> Array<Tuple<Optional<ITerm>, float>, N>
{
    float4 sumOver(float4 u)
    {
        float4 v = 0.f;
        for (var i = 0; i < N; i++)
        {
            var term = this[i];
            if (let t = term._0)
                v += t.evaluate(u) * term._1;
        }
        return v;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID) {

    let pixel = uint2(threadId.x, threadId.y);
    Array<Tuple<Optional<ITerm>, float>, 3> terms1 =
        { 
            {Optional<ITerm>(Multiply(4.f)), 0.5f}, 
            {Optional<ITerm>(Pow(2)), 0.5f}, 
            {none, 0.f} 
        };

    // 4 * 0.5 * 0.5 + (0.5 ^ 2) * 0.5 = 1 + 0.125 = 1.125
    outputBuffer[0] = terms1.sumOver(float4(0.5)).x; // CHECK: 1.125

    Array<Tuple<Optional<ITerm>, float>, 4> terms2 = 
        { 
            {Optional<ITerm>(Multiply(3.f)), 1.f}, 
            {none, 0.f}, 
            {Optional<ITerm>(Multiply(4.f)), 0.5f}, 
            {Optional<ITerm>(Multiply(5.f)), 0.5f} 
        };

    // 3 * 0.5 + 4 * 0.5 * 0.5 + 5 * 0.5 * 0.5 = 1.5 + 1 + 1.25 = 3.75
    outputBuffer[1] = terms2.sumOver(float4(0.5)).x; // CHECK: 3.75
}