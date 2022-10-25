//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef __DifferentialPair<float> dpfloat;

typealias IDFloat = IFloat & IDifferentiable;

// Differential standard library

namespace dstd
{
    __generic<T : IDFloat>
    __target_intrinsic(hlsl)
    __target_intrinsic(glsl)
    __target_intrinsic(cuda, "$P_exp($0)")
    __target_intrinsic(cpp, "$P_exp($0)")
    __target_intrinsic(spirv_direct, "12 resultType resultId glsl450 27 _0")
    [__custom_jvp(d_exp)]
    T exp(T x);

    __generic<T : IDFloat>
    __DifferentialPair<T> d_exp(__DifferentialPair<T> dpx)
    {
        return __DifferentialPair<T>(
            exp(dpx.p()),
            T.dmul(exp(dpx.p()), dpx.d()));
    }
};

__differentiate_jvp float f(float x)
{
    return dstd.exp(x);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    {
        dpfloat dpa = dpfloat(2.0, 1.0);

        outputBuffer[0] = f(dpa.p());        // Expect: 22.0
        outputBuffer[1] = __jvp(f)(dpa).d(); // Expect: 9.5
    }
}