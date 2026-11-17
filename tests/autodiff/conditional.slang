//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -output-using-type
//TEST(compute,vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -slang -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

// Conditional payloads marked no_diff are constant context. Reading them must
// still work in differentiated code.
[Differentiable]
float3 multiply(no_diff Conditional<float3, true> normal, float3 v)
{
    if (let n = normal.get())
        return n * v;
    return v;
}

struct Model<let mask : uint> : IDifferentiable
{
    float base;
    Conditional<float, (mask & 1u) != 0u> optional =
        Conditional<float, (mask & 1u) != 0u>(0.0);
}

[Differentiable]
float evaluate<let mask : uint>(Model<mask> m)
{
    if (let v = m.optional.get())
        return m.base * v;
    return m.base;
}

[Differentiable]
float constructAndSet<let enabled : bool>(no_diff float scale, float x)
{
    Conditional<float, enabled> c = scale;
    c.set(scale + 1.0);
    if (let value = c.get())
        return value * x * x;
    return x * x;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Conditional<float3, true> normal = float3(2.0);
    let forward = fwd_diff(multiply)(normal, diffPair(float3(1.0), float3(1.0)));
    // CHECK: 2.0
    // CHECK: 2.0
    // CHECK: 2.0
    outputBuffer[0] = forward.d.x;
    outputBuffer[1] = forward.d.y;
    outputBuffer[2] = forward.d.z;
    var v = diffPair(float3(1.0), float3(0.0));
    bwd_diff(multiply)(normal, v, float3(1.0));
    // CHECK: 2.0
    // CHECK: 2.0
    // CHECK: 2.0
    outputBuffer[3] = v.d.x;
    outputBuffer[4] = v.d.y;
    outputBuffer[5] = v.d.z;

    Model<1u> enabled = {};
    enabled.base = 2.0;
    enabled.optional.set(3.0);
    Model<1u>.Differential dEnabled = {};
    dEnabled.base = 1.0;
    // CHECK: 3.0
    outputBuffer[6] = fwd_diff(evaluate<1u>)(diffPair(enabled, dEnabled)).d;

    Model<0u> disabled = {};
    disabled.base = 2.0;
    Model<0u>.Differential dDisabled = {};
    dDisabled.base = 1.0;
    // CHECK: 1.0
    outputBuffer[7] = fwd_diff(evaluate<0u>)(diffPair(disabled, dDisabled)).d;

    // CHECK: 24.0
    outputBuffer[8] = fwd_diff(constructAndSet<true>)(3.0, diffPair(3.0, 1.0)).d;
    var x = diffPair(3.0, 0.0);
    bwd_diff(constructAndSet<true>)(3.0, x, 1.0);
    // CHECK: 24.0
    outputBuffer[9] = x.d;

    // CHECK: 6.0
    outputBuffer[10] = fwd_diff(constructAndSet<false>)(3.0, diffPair(3.0, 1.0)).d;
    x = diffPair(3.0, 0.0);
    bwd_diff(constructAndSet<false>)(3.0, x, 1.0);
    // CHECK: 6.0
    outputBuffer[11] = x.d;
}
