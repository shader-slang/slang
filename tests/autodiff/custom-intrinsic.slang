//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef __DifferentialPair<float> dpfloat;

typealias IDFloat = IFloat & IDifferentiable;

namespace myintrinsiclib
{
    __generic<T : IDFloat>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    __target_intrinsic(cuda, "$P_exp($0)")
    __target_intrinsic(cpp, "$P_exp($0)")
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 27 _0")
    [__custom_jvp(d_exp<T>)]
    T exp(T x);

    __generic<T : IDFloat>
    __DifferentialPair<T> d_exp(__DifferentialPair<T> dpx)
    {
        return __DifferentialPair<T>(
            exp(dpx.p()),
            T.dmul(exp(dpx.p()), dpx.d()));
    }

    
    // Sine
    __generic<T : IDFloat>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    __target_intrinsic(cuda, "$P_sin($0)")
    __target_intrinsic(cpp, "$P_sin($0)")
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 13 _0")
    [__custom_jvp(d_sin<T>)]
    T sin(T x);

    __generic<T : IDFloat>
    __DifferentialPair<T> d_sin(__DifferentialPair<T> dpx)
    {
        return __DifferentialPair<T>(
            sin(dpx.p()),
            T.dmul(cos(dpx.p()), dpx.d()));
    }

    // Cosine
    __generic<T : IDFloat>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    __target_intrinsic(cuda, "$P_cos($0)")
    __target_intrinsic(cpp, "$P_cos($0)")
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 14 _0")
    [__custom_jvp(d_cos<T>)]
    T cos(T x);

    __generic<T : IDFloat>
    __DifferentialPair<T> d_cos(__DifferentialPair<T> dpx)
    {
        return __DifferentialPair<T>(
            cos(dpx.p()),
            T.dmul(-sin(dpx.p()), dpx.d()));
    }

    // Sine and cosine
    __generic<T : IDFloat>
    __target_intrinsic(hlsl)
    __target_intrinsic(cuda, "$P_sincos($0, $1, $2)")
    [__custom_jvp(d_sincos<T>)]
    void sincos(T x, out T s, out T c)
    {
        s = sin(x);
        c = cos(x);
    }

    __generic<T : IDFloat>
    void d_sincos(__DifferentialPair<T> x, out __DifferentialPair<T> s, out __DifferentialPair<T> c)
    {
        T _s;
        T _c;
        sincos(x.p(), _s, _c);

        s = __DifferentialPair<T>(_s, T.dmul(_c, x.d()));
        c = __DifferentialPair<T>(_c, T.dmul(-_s, x.d()));
    }
};

__differentiate_jvp float f(float x)
{
    return myintrinsiclib.exp(x);
}

__differentiate_jvp float g(float x)
{
    float s;
    float t;
    myintrinsiclib.sincos(x, s, t);

    return s + t;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    {
        dpfloat dpa = dpfloat(2.0, 1.0);

        outputBuffer[0] = f(dpa.p());        // Expect: 7.389056
        outputBuffer[1] = __jvp(f)(dpa).d(); // Expect: 7.389056

        // g() needs additional handling of  IRMakeDifferentialPair(PtrType). This needs to 
        // generate a new var, load from the individual vars and store into the pair var.

        //outputBuffer[2] = g(dpa.p());        // Expect: 1.381773
        //outputBuffer[3] = __jvp(g)(dpa).d(); // Expect: -0.301168
    }
}