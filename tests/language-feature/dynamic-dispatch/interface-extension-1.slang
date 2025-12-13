//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IInterface
{
    float calc(float x);
}

struct A : IInterface
{
    float calc(float x) { return x * x * x; }
};

struct B : IInterface
{
    float calc(float x) { return x * x; }
};

struct C : IInterface
{
    float calc(float x) { return x; }
};

extension<T : IInterface> T
{
    float calcTwice(float x)
    {
        return this.calc(x) + this.calc(x);
    }
}

IInterface factoryAB(uint id, float x)
{
    if (id == 0)
        return A();
    else
        return B();
}

float calcTwice(IInterface obj, float y)
{
    return obj.calcTwice(y);
}

// Statically resolvable calls to calcTwice
float f(uint id, float x)
{
    return calcTwice(A(), x) + calcTwice(B(), x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = f(0, 3); // CHECK: 72
}