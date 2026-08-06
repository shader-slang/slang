//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

// Regression test for bwd_diff(value.method) after bwd_diff became a static
// associated function. The object value must be passed explicitly as argument 0.

interface IShape3D
{
    [Differentiable]
    [NoDiffThis]
    float distance(float3 p);
}

struct Sphere : IShape3D
{
    float r;

    [Differentiable]
    [NoDiffThis]
    float distance(float3 p)
    {
        return length(p) - r;
    }
}

float3 gradient3<S : IShape3D>(S shape, float3 p)
{
    var dp = diffPair(p);

    // Generic/interface case: the first argument supplies the concrete generic
    // object value `shape`, followed by the original method parameters.
    bwd_diff(shape.distance)(shape, dp, 1.0);
    return dp.d;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Sphere s = {1.0};
    float3 p = float3(3.0, 0.0, 0.0);
    var dp = diffPair(p);

    // Concrete case: `s.distance` still requires `s` as the first explicit
    // argument for the synthesized static backward derivative.
    bwd_diff(s.distance)(s, dp, 1.0);
    float3 concreteGradient = dp.d;
    float3 genericGradient = gradient3(s, p);

    outputBuffer[0] = concreteGradient.x;
    outputBuffer[1] = concreteGradient.y;
    outputBuffer[2] = concreteGradient.z;
    outputBuffer[3] = genericGradient.x;
    outputBuffer[4] = genericGradient.y;
    outputBuffer[5] = genericGradient.z;

    // CHECK: 1.0
    // CHECK: 0.0
    // CHECK: 0.0
    // CHECK: 1.0
    // CHECK: 0.0
    // CHECK: 0.0
}
