//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef float Real;

typealias IDFloat = IFloat & IDifferentiable;

__generic<T, let N : int>
struct dvector
{
    // T values[N];
    vector<T, N> values;
};

__generic<T : IDFloat, let N : int>
struct myvector : IDifferentiable
{
    // T values[N];
    vector<T, N> values;
    typedef dvector<T.Differential, N> Differential;

    /*
    [__unsafeForceInlineEarly]
    static T.Differential __getDifferentialAtIndexFor_values(dvector<T.Differential, N> d, uint32_t index)
    {
        return d.values[index];
    }

    [__unsafeForceInlineEarly]
    static void __setDifferentialAtIndexFor_values(inout dvector<T.Differential, N> d, T.Differential v, uint32_t index)
    {
        d.values[index] = v;
    }
    */

    [__unsafeForceInlineEarly]
    static vector<T.Differential, N> __getDifferentialFor_values(dvector<T.Differential, N> d)
    {
        return d.values;
    }

    [__unsafeForceInlineEarly]
    static void __setDifferentialFor_values(inout dvector<T.Differential, N> d, vector<T.Differential, N> v)
    {
        d.values = v;
    }

    __init(T c)
    {
        for (int i = 0; i < N; i++)
        {
            values[i] = c;
        }
    }

    static Differential dadd(Differential a, Differential b)
    {
        Differential output;

        for (int i = 0; i < N; i++)
        {
            output.values[i] = T.dadd(a.values[i], b.values[i]);
        }

        return output;
    }

    
    static Differential dmul(This a, Differential b)
    {
        Differential output;
        
        for (int i = 0; i < N; i++)
        {
            output.values[i] = T.dmul(a.values[i], b.values[i]);
        }

        return output;
    }

    static Differential zero()
    {
        Differential output;

        for (int i = 0; i < N; i++)
        {
            output.values[i] = T.zero();
        }

        return output;
    }
};

__generic<T : IDFloat, let N : int>
__differentiate_jvp myvector<T, N> operator +(myvector<T, N> a, myvector<T, N> b)
{
    myvector<T, N> output;
    for (int i = 0; i < N; i++)
    {
        output.values[i] = a.values[i] + b.values[i];
    }
    return output;
}

__generic<T : IDFloat, let N : int>
    __differentiate_jvp myvector<T, N> operator *(myvector<T, N> a, myvector<T, N> b)
{
    myvector<T, N> output;
    for (int i = 0; i < N; i++)
    {
        output.values[i] = a.values[i] * b.values[i];
    }
    return output;
}

__generic<T : IDFloat, let N : int>
    __differentiate_jvp myvector<T, N> operator *(T a, myvector<T, N> b)
{
    myvector<T, N> output;
    for (int i = 0; i < N; i++)
    {
        output.values[i] = a * b.values[i];
    }
    return output;
}

__generic<T : IDFloat, let N : int>
[__custom_jvp(dot_jvp)]
T dot(myvector<T, N> a, myvector<T, N> b)
{
    T curr = (T)0.0;
    for (int i = 0; i < N; i++)
    {
        curr = curr + (a.values[i] * b.values[i]);
    }

    return curr;
}

__generic<T : IDFloat, let N : int>
typedef __DifferentialPair<myvector<T, N>> dpvector;

__generic<T : IDFloat, let N : int>
__DifferentialPair<T> dot_jvp(dpvector<T, N> a, dpvector<T, N> b)
{
    T.Differential curr_d = (T.zero());
    T curr_p = (T)0.0;
    for (int i = 0; i < N; i++)
    {
        curr_p = curr_p + (a.p().values[i] * b.p().values[i]);
        curr_d = T.dadd(
                    curr_d, 
                    T.dadd(
                        T.dmul(a.p().values[i], b.d().values[i]),
                        T.dmul(b.p().values[i], a.d().values[i])));
    }

    return __DifferentialPair<T>(curr_p, curr_d);
}

__generic<let N : int>
struct lineardvector
{
     myvector<Real, N>.Differential val;
};

__generic<let N : int>
struct linearvector : MyLinearArithmeticType, IDifferentiable
{
    typedef lineardvector<N> Differential;

    myvector<Real, N> val;

    static myvector<Real, N>.Differential __getDifferentialForVal(lineardvector<N> dvec)
    {
        return dvec.val;
    }

    static void __setDifferentialForVal(lineardvector<N> dvec, myvector<Real, N>.Differential v)
    {
        dvec.val = v;
    }

    static __differentiate_jvp linearvector<N> ladd(linearvector<N> a, linearvector<N> b)
    {
        return linearvector<N>(a.val + b.val);
    }

    static __differentiate_jvp linearvector<N> lmul(linearvector<N> a, linearvector<N> b)
    {
        return linearvector<N>(a.val * b.val);
    }

    static __differentiate_jvp linearvector<N> lscale(float a, linearvector<N> b)
    {
        return linearvector<N>(a * b.val);
    }

    static __differentiate_jvp float ldot(linearvector<N> a, linearvector<N> b)
    {
        return dot(a.val, b.val);
    }

    static Differential zero()
    {
        return linearvector<N>(myvector<Real, N>((Real)0.0));
    }

    static Differential dadd(Differential a, Differential b)
    {
        return myvector<Real, N>.dadd(a.val, b.val);
    }
    
    static Differential dmul(This a, Differential b)
    {
        return myvector<Real, N>.dmul(a.val, b.val);
    }

    __differentiate_jvp __init(vector<Real, N> a)
    {
        for (int i = 0; i < N; i++)
        {
            val.values[i] = a[i];
        }
    }

    __differentiate_jvp __init(myvector<Real, N> a)
    {
        val = a;
    }
};

typedef linearvector<3> myfloat3;
typedef linearvector<4> myfloat4;

typedef __DifferentialPair<Real> dpfloat;

interface MyLinearArithmeticType
{
    static This ladd(This a, This b);
    static This lmul(This a, This b);
    static This lscale(Real a, This b);
    static Real ldot(This a, This b);
};

typedef __DifferentialPair<myfloat4> dpfloat4;
typedef __DifferentialPair<myfloat3> dpfloat3;

extension float : MyLinearArithmeticType
{
    static __differentiate_jvp float ladd(float a, float b)
    {
        return a + b;
    }

    static __differentiate_jvp float lmul(float a, float b)
    {
        return a * b;
    }

    static __differentiate_jvp float lscale(float a, float b)
    {
        return a * b;
    }

    static __differentiate_jvp float ldot(float a, float b)
    {
        return a * b;
    }
};

typealias MyLinearArithmeticDifferentiableType = IDifferentiable & MyLinearArithmeticType;

__generic<T : MyLinearArithmeticDifferentiableType>
__differentiate_jvp T operator +(T a, T b)
{
    return T.ladd(a, b);
}

__generic<T : MyLinearArithmeticDifferentiableType>
__differentiate_jvp T operator *(T a, T b)
{
    return T.lmul(a, b);
}

__generic<G : MyLinearArithmeticDifferentiableType>
__differentiate_jvp G f(G x)
{
    G a = x + x;
    G b = x * x;

    return a * a + G.lscale((Real)3.0, x); 
}


[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    {
        dpfloat dpa = dpfloat(2.0, 1.0);
        dpfloat4 dpf4 = dpfloat4(myfloat4(float4(1.5, 2.0, 0.5, 1.0)), myfloat4(float4(0.5, 0.8, 1.6, 2.5)));
        dpfloat3 dpf3 = dpfloat3(myfloat3(float3(1.0, 3.0, 5.0)), myfloat3(float3(0.5, 1.5, 2.5)));

        outputBuffer[0] = f(dpa.p());                               // Expect: 22.0
        outputBuffer[1] = __jvp(f)(dpfloat(2.0, 0.5)).d();          // Expect: 9.5
        outputBuffer[2] = __jvp(f)(dpf4).d().val.values[3];         // Expect: 27.5
        outputBuffer[3] = __jvp(f)(dpf3).d().val.values[1];         // Expect: 40.5
    }
}
