//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;

typealias IDFloat = __BuiltinFloatingPointType & IDifferentiable;

namespace myintrinsiclib
{
    __generic<T : IDFloat>
    [ForwardDerivative(d_myexp<T>)]
    T myexp(T x)
    {
        __target_switch
        {
            case cpp: __intrinsic_asm "$P_exp($0)";
            case cuda: __intrinsic_asm "$P_exp($0)";
            case glsl: __intrinsic_asm "exp";
            case hlsl: __intrinsic_asm "exp";
            case metal: __intrinsic_asm "exp";
            case spirv: return spirv_asm {
                    OpExtInst $$T result glsl450 27 $x
            };
            case wgsl: __intrinsic_asm "exp";
            case llvm: __intrinsic_asm "%result = call $T0 @llvm.exp($0)";
        }
    }

    __generic<T : IDFloat>
    DifferentialPair<T> d_myexp(DifferentialPair<T> dpx)
    {
        return DifferentialPair<T>(
            myexp(dpx.p),
            T.dmul(myexp(dpx.p), dpx.d));
    }

    
    // Sine
    __generic<T : IDFloat>
    [ForwardDerivative(d_mysin<T>)]
    T mysin(T x)
    {
        __target_switch
        {
            case hlsl: __intrinsic_asm "sin($0)";
            case glsl: __intrinsic_asm "sin($0)";
            case metal: __intrinsic_asm "sin($0)";
            case cuda: __intrinsic_asm "$P_sin($0)";
            case cpp: __intrinsic_asm "$P_sin($0)";
            case spirv: return spirv_asm {
                    OpExtInst $$T result glsl450 13 $x
            };
            case wgsl: __intrinsic_asm "sin($0)";
            case llvm: __intrinsic_asm "%result = call $T0 @llvm.sin($0)";
        }
    }

    __generic<T : IDFloat>
    DifferentialPair<T> d_mysin(DifferentialPair<T> dpx)
    {
        return DifferentialPair<T>(
            mysin(dpx.p),
            T.dmul(mycos(dpx.p), dpx.d));
    }

    // Cosine
    __generic<T : IDFloat>
    [ForwardDerivative(d_mycos<T>)]
    T mycos(T x)
    {
        __target_switch
        {
            case hlsl: __intrinsic_asm "cos($0)";
            case glsl: __intrinsic_asm "cos($0)";
            case metal: __intrinsic_asm "cos($0)";
            case cuda: __intrinsic_asm "$P_cos($0)";
            case cpp: __intrinsic_asm "$P_cos($0)";
            case spirv: return spirv_asm {
                OpExtInst $$T result glsl450 14 $x
            };
            case wgsl: __intrinsic_asm "cos($0)";
            case llvm: __intrinsic_asm "%result = call $T0 @llvm.cos($0)";
        }
    }

    __generic<T : IDFloat>
    DifferentialPair<T> d_mycos(DifferentialPair<T> dpx)
    {
        return DifferentialPair<T>(
            mycos(dpx.p),
            T.dmul(-sin(dpx.p), dpx.d));
    }

    // Sine and cosine
    __generic<T : IDFloat>
    __target_intrinsic(hlsl, "sincos($0, $1, $2)")
    __target_intrinsic(cuda, "$P_sincos($0, $1, $2)")
    [ForwardDerivative(d_mysincos<T>)]
    void mysincos(T x, out T s, out T c)
    {
        s = sin(x);
        c = cos(x);
    }

    __generic<T : IDFloat>
    void d_mysincos(DifferentialPair<T> x, out DifferentialPair<T> s, out DifferentialPair<T> c)
    {
        T _s;
        T _c;
        mysincos(x.p, _s, _c);

        s = DifferentialPair<T>(_s, T.dmul(_c, x.d));
        c = DifferentialPair<T>(_c, T.dmul(-_s, x.d));
    }
};

[ForwardDifferentiable]
float f(float x)
{
    return myintrinsiclib.myexp(x);
}

[ForwardDifferentiable]
float g(float x)
{
    float s;
    float t;
    myintrinsiclib.mysincos(x, s, t);

    return s + t;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    {
        dpfloat dpa = dpfloat(2.0, 1.0);

        outputBuffer[0] = f(dpa.p);        // Expect: 7.389056
        outputBuffer[1] = __fwd_diff(f)(dpa).d; // Expect: 7.389056

        // g() needs additional handling of  IRMakeDifferentialPair(PtrType). This needs to 
        // generate a new var, load from the individual vars and store into the pair var.

        //outputBuffer[2] = g(dpa.p);        // Expect: 1.381773
        //outputBuffer[3] = __fwd_diff(g)(dpa).d; // Expect: -0.301168
    }
}
