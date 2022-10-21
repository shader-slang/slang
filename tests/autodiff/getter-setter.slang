//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct B
{
    float z;
};

struct A : IDifferentiable
{
    typedef B Differential;
    
    float _x;
    float _y;

    property float x
    {
        get { return _x; }
        set { _x = newValue; }
        dget { return differential.z; }
        dset { differential.z = newValue; }
    }

    property float y
    {
        get { return _y; }
        set { _y = newValue; }
    }

    [__unsafeForceInlineEarly]
    static Differential zero()
    {
        B b = {0.0};
        return b;
    }

    [__unsafeForceInlineEarly]
    static Differential dadd(Differential a, Differential b)
    {
        B o = {a.z + b.z};
        return o;
    }

    [__unsafeForceInlineEarly]
    static Differential dmul(This a, Differential b)
    {
        B o = {a.x * b.z};
        return o;
    }
};

typedef __DifferentialPair<A> dpA;

__differentiate_jvp A f(A a)
{
    A aout;
    aout.y = 2 * a.x;
    aout.x = 5 * a.x;

    return aout;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    {
        A a = {1.0, 2.0};
        B b = {0.2};

        dpA dpa = dpA(a, b);

        outputBuffer[0] = __jvp(f)(dpa).d().z;                    // Expect: 1
    }
}
