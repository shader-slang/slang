//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef float Real;

typealias IDFloat = __BuiltinFloatingPointType;

struct dvector<T : IDifferentiable, let N : int> : IDifferentiable
{
    typedef dvector<T.Differential, N> Differential;
    [DerivativeMember(Differential.values)]
    T values[N];
};

struct myvector<T : IDFloat, let N : int> : IDifferentiable
{
    typedef dvector<T.Differential, N> Differential;

    [DerivativeMember(Differential.values)]
    T values[N];
    
    [Differentiable]
    __init(T c)
    {
        [ForceUnroll]
        for (int i = 0; i < N; i++)
        {
            values[i] = c;
        }
    }

    [Differentiable]
    static Differential dadd(Differential a, Differential b)
    {
        Differential output;

        [ForceUnroll]
        for (int i = 0; i < N; i++)
        {
            output.values[i] = T.dadd(a.values[i], b.values[i]);
        }

        return output;
    }

    [Differentiable]
    static Differential dzero()
    {
        Differential output;

        [ForceUnroll]
        for (int i = 0; i < N; i++)
        {
            output.values[i] = T.dzero();
        }

        return output;
    }
};

[Differentiable]
__generic<T : IDFloat, let N : int>
myvector<T, N> operator +(myvector<T, N> a, myvector<T, N> b)
{
    myvector<T, N> output;
    [ForceUnroll]
    for (int i = 0; i < N; i++)
    {
        output.values[i] = a.values[i] + b.values[i];
    }
    return output;
}

[Differentiable]
__generic<T : IDFloat, let N : int>
myvector<T, N> operator *(myvector<T, N> a, myvector<T, N> b)
{
    myvector<T, N> output;
    [ForceUnroll]
    for (int i = 0; i < N; i++)
    {
        output.values[i] = a.values[i] * b.values[i];
    }
    return output;
}

[Differentiable]
__generic<T : IDFloat, let N : int>
myvector<T, N> operator *(T a, myvector<T, N> b)
{
    myvector<T, N> output;
    [ForceUnroll]
    for (int i = 0; i < N; i++)
    {
        output.values[i] = a * b.values[i];
    }
    return output;
}

[ForwardDerivative(dot_fwd_diff)]
[BackwardDerivative(dot_bwd_diff)]
T dot<T : IDFloat, let N : int>(myvector<T, N> a, myvector<T, N> b)
{
    T curr = __realCast<T, float>(0.f);
    [ForceUnroll]
    for (int i = 0; i < N; i++)
    {
        curr = curr + (a.values[i] * b.values[i]);
    }

    return curr;
}

__generic<T : IDFloat, let N : int>
typedef DifferentialPair<myvector<T, N>> dpvector;

DifferentialPair<T> dot_fwd_diff<T : IDFloat, let N : int>(dpvector<T, N> a, dpvector<T, N> b)
{
    T.Differential curr_d = (T.dzero());
    T curr_p = __realCast<T, float>(0.f);
    [ForceUnroll]
    for (int i = 0; i < N; i++)
    {
        curr_p = curr_p + (a.p.values[i] * b.p.values[i]);
        curr_d = T.dadd(
                    curr_d, 
                    T.dadd(
                        __mul_p_d(a.p.values[i], b.d.values[i]),
                        __mul_p_d(b.p.values[i], a.d.values[i])));
    }

    return DifferentialPair<T>(curr_p, curr_d);
}

void dot_bwd_diff<T : IDFloat, let N : int>(inout dpvector<T, N> dpa, inout dpvector<T, N> dpb, T.Differential dOut)
{
    //dpvector<T, N> newDpa = dpa;
    //dpvector<T, N> newDpb = dpb;

    var dA = myvector<T, N>::dzero();
    var dB = myvector<T, N>::dzero();

    [ForceUnroll]
    for (int i = 0; i < N; i++)
    {
        dA.values[i] = __mul_p_d(dpb.p.values[i], dOut);
        dB.values[i] = __mul_p_d(dpa.p.values[i], dOut);
    }

    dpa = diffPair(dpa.p, dA);
    dpb = diffPair(dpb.p, dB);
}


struct lineardvector<let N : int> : IDifferentiable
{
    typedef lineardvector<N> Differential;

    myvector<Real, N>.Differential val;

    [Differentiable]
    __init(vector<Real.Differential, N> a)
    {
        [ForceUnroll]
        for (int i = 0; i < N; i++)
        {
            val.values[i] = a[i];
        }
    }

    // Add a new constructor for dadd() function.
    [Differentiable]
    __init(Real a[N])
    {
        [ForceUnroll]
        for (int i = 0; i < N; i++)
        {
            val.values[i] = a[i];
        }
    }
};


struct linearvector<let N : int> : MyLinearArithmeticType, IDifferentiable
{
    typedef lineardvector<N> Differential;

    [DerivativeMember(Differential.val)]
    myvector<Real, N> val;

    [Differentiable]
    static linearvector<N> ladd(linearvector<N> a, linearvector<N> b)
    {
        return linearvector<N>(a.val + b.val);
    }

    [Differentiable]
    static linearvector<N> lmul(linearvector<N> a, linearvector<N> b)
    {
        return linearvector<N>(a.val * b.val);
    }

    [Differentiable]
    static linearvector<N> lscale(float a, linearvector<N> b)
    {
        return linearvector<N>(a * b.val);
    }

    [Differentiable]
    static float ldot(linearvector<N> a, linearvector<N> b)
    {
        return dot(a.val, b.val);
    }

    [Differentiable]
    static Differential dzero()
    {
        lineardvector<N> dout;
        dout.val = myvector<Real, N>.dzero();
        return dout;
    }

    [Differentiable]
    static Differential dadd(Differential a, Differential b)
    {
        // return { myvector<Real, N>.dadd(a.val, b.val) };
        //
        // Above code will not work because
        // myvector<Real, N>.dadd will return dvector<T.Differential, N> type
        // while Differential == lineardvector<N> type
        // and the constructor of lineardvector<N> requires a vector<Real.Differential, N> type
        // and dvector<T.Differential, N> != vector<Real.Differential, N>, though they have the
        // same members.
        //
        // In our new design, generic will not be C-Style struct anymore.
        dvector<Real.Differential, N> d = myvector<Real, N>.dadd(a.val, b.val);
        return {d.values};
    }

    [Differentiable]
    __init(vector<Real, N> a)
    {
        [ForceUnroll]
        for (int i = 0; i < N; i++)
        {
            val.values[i] = a[i];
        }
    }

    [Differentiable]
    __init(myvector<Real, N> a)
    {
        val = a;
    }
};

typedef linearvector<3> myfloat3;
typedef linearvector<4> myfloat4;

typedef lineardvector<3> mydfloat3;
typedef lineardvector<4> mydfloat4;

typedef DifferentialPair<Real> dpfloat;

interface MyLinearArithmeticType
{
    [Differentiable]
    static This ladd(This a, This b);

    [Differentiable]
    static This lmul(This a, This b);

    [Differentiable]
    static This lscale(Real a, This b);

    [Differentiable]
    static Real ldot(This a, This b);
};

typedef DifferentialPair<myfloat4> dpfloat4;
typedef DifferentialPair<myfloat3> dpfloat3;

extension float : MyLinearArithmeticType
{
    [Differentiable]
    static float ladd(float a, float b)
    {
        return a + b;
    }

    [Differentiable]
    static float lmul(float a, float b)
    {
        return a * b;
    }

    [Differentiable]
    static float lscale(float a, float b)
    {
        return a * b;
    }

    [Differentiable]
    static float ldot(float a, float b)
    {
        return a * b;
    }
};

typealias MyLinearArithmeticDifferentiableType = IDifferentiable & MyLinearArithmeticType;

__generic<T : MyLinearArithmeticDifferentiableType>
[Differentiable]
T operator +(T a, T b)
{
    return T.ladd(a, b);
}

__generic<T : MyLinearArithmeticDifferentiableType>
[Differentiable]
T operator *(T a, T b)
{
    return T.lmul(a, b);
}

[Differentiable]
G f<G : MyLinearArithmeticDifferentiableType>(G x)
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
        dpfloat4 dpf4 = dpfloat4(myfloat4(float4(1.5, 2.0, 0.5, 1.0)), mydfloat4(float4(0.5, 0.8, 1.6, 2.5)));
        dpfloat3 dpf3 = dpfloat3(myfloat3(float3(1.0, 3.0, 5.0)), mydfloat3(float3(0.5, 1.5, 2.5)));

        outputBuffer[0] = f(dpa.p);                               // Expect: 22.0
        outputBuffer[1] = fwd_diff(f)(dpfloat(2.0, 0.5)).d;          // Expect: 9.5
        outputBuffer[2] = fwd_diff(f)(dpf4).d.val.values[3];         // Expect: 27.5
        outputBuffer[3] = fwd_diff(f)(dpf3).d.val.values[1];         // Expect: 40.5
    }
}
