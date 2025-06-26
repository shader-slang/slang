//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Test for lowering witness information within function bodies for synthesis.

[Differentiable]
T zig<T : IFloat>(T x)
{
    return x * x;
}

[Differentiable]
T zag<T : IFloat>(T x)
{
    return x + (T)1;
}

[Differentiable]
T zap<T : IFloat>(T x)
{
    return (T)2 * zag(zig(x));
}

extension<T : IFloat> __func_as_type(zap<T>) : IBackwardDifferentiable<__func_as_type(zap<T>)>, IForwardDifferentiable<__func_as_type(zap<T>)> { };
extension<T : IFloat> __func_as_type(zig<T>) : IBackwardDifferentiable<__func_as_type(zig<T>)>, IForwardDifferentiable<__func_as_type(zig<T>)> { };
extension<T : IFloat> __func_as_type(zag<T>) : IBackwardDifferentiable<__func_as_type(zag<T>)>, IForwardDifferentiable<__func_as_type(zag<T>)> { };

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    var zapBwdPass1 = zap<float>.apply_bwd(2.f);
    var zapBwdPass2 = zap<float2>.apply_bwd(float2(3.f, 5.f));
    var zapBwdPass3 = zap<float3>.apply_bwd(float3(4.f, 5.f, 6.f));

    outputBuffer[0] = zapBwdPass1.val();
    outputBuffer[1] = zapBwdPass2.val().y;
    outputBuffer[2] = zapBwdPass3.val().z;

    float dx = 0.f;
    zapBwdPass1(dx, 1.f);
    outputBuffer[3] = dx;

    float2 dx2 = 0.f;
    zapBwdPass2(dx2, float2(1.f, 1.f));
    outputBuffer[4] = dx2.y;

    float3 dx3 = 0.f;
    zapBwdPass3(dx3, float3(1.f, 1.f, 1.f));
    outputBuffer[5] = dx3.z;
}
