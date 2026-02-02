//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -Xslang -Wno-41020

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//
// Test tuples of interface types with some concrete elements and
// as struct fields
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

struct MyComputation
{
    Tuple<ITerm, ITerm, float> terms;
    float offset;

    float4 doThingOne(float4 u)
    {
        return ((terms._0.evaluate(u) - terms._1.evaluate(u)) * terms._2) + offset;
    }

    float4 doThingTwo(float4 u)
    {
        return ((terms._0.evaluate(u) + terms._1.evaluate(u)) * terms._2) + offset;
    }
}

float4 sumThingOneAndTwo(Tuple<ITerm, ITerm, ITerm, ITerm, float> terms, float4 u)
{
    // comp1 = { (Multiply(4.f), Multiply(20.f), 1.5f), 0.5f }
    MyComputation comp1 = { terms._0_1_4, 0.5f };

    // comp2 = { (Multiply(10.f), Multiply(4.f), 1.5f), 1.0f }
    MyComputation comp2 = { terms._2_3_4, 1.0f };

    // = ((4 * 0.5 + 20 * 0.5) * 1.5 + 0.5) + ((10 * 0.5 - 4 * 0.5) * 1.5 + 1.0)
    // = (2 + 10) * 1.5 + 0.5 + (5 - 2) * 1.5 + 1.0
    // = 12 * 1.5 + 0.5 + 3 * 1.5 + 1.0
    // = 18 + 0.5 + 4.5 + 1.0
    // = 24.0
    return comp1.doThingTwo(u) + comp2.doThingOne(u);
}

void modify(inout Tuple<ITerm, ITerm, ITerm, ITerm, float> terms)
{
    terms._2_1 = makeTuple<ITerm, ITerm>(Multiply(10.f), Multiply(20.f));
    terms._4 *= 0.5f;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID) {

    let pixel = uint2(threadId.x, threadId.y);
    Tuple<ITerm, ITerm, ITerm, ITerm, float> terms = { Multiply(4.f), Pow(3), Multiply(3.f), Multiply(4.f), 3.f };

    modify(terms);

    // After modify:
    // tuple = { Multiply(4.f), Multiply(20.f), Multiply(10.f), Multiply(4.f), 1.5f }
    outputBuffer[0] = sumThingOneAndTwo(terms, float4(0.5)).x; // CHECK: 24.0
}