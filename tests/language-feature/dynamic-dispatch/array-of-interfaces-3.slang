//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -Xslang -Wno-41020

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//
// Test struct fields with array of optional interface types, with multiple
// implementations for the interface.
// 

public interface Term 
{
    float4 evaluate(float4 u);
}

public struct Sum 
{
    Optional<Term> terms[16];
    uint count;

    public __init() 
    { 
        count = 0; 
        for (uint i = 0; i < 16; i++) 
            terms[i] = none;
    }

    [mutating]
    public void add(Term term) 
    {
        terms[count] = term;
        count += 1;
    }

    public float4 evaluate(float4 u) 
    { 
        float4 val = 0.f;

        for (uint i = 0; i < count; i++)
        {
            if (let termOpt = terms[i])
                val += termOpt.evaluate(u);
        }
        
        return val;
    }
}

public struct Multiply : Term
{
    float factor;
    public __init(float factor) { this.factor = factor; }
    public float4 evaluate(float4 u) { return u * factor; }
}

public struct Pow : Term
{
    int exponent;
    public __init(int exponent) { this.exponent = exponent; }
    public float4 evaluate(float4 u) { return pow(u.x, exponent); }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID) 
{
    {
        Sum terms;
        terms.add(Multiply(3.f));
        terms.add(Pow(2));
        
        // 0.5 * 3 + (0.5 ^ 2) = 1.5 + 0.25 = 1.75
        outputBuffer[0] = terms.evaluate(float4(0.5)).x; // CHECK: 1.75
    }

    {
        Sum terms;
        terms.add(Multiply(4.f));
        terms.add(Pow(3));
        terms.add(Pow(4));

        // 0.5 * 4 + (0.5 ^ 3) + (0.5 ^ 4) = 2.0 + 0.125 + 0.0625 = 2.1875
        outputBuffer[1] = terms.evaluate(float4(0.5)).x; // CHECK: 2.1875
    }
}