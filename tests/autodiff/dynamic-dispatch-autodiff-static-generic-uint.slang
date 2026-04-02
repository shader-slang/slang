// Test calling a static differentiable generic requirement through dynamic dispatch,
// where the generic value parameter is `uint`.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-d3d12 -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[anyValueSize(16)]
interface IInterface
{
    [Differentiable]
    static float calc<let N : uint>(vector<float, N> v);
}

struct A : IInterface
{
    [Differentiable]
    static float calc<let N : uint>(vector<float, N> v)
    {
        float sum = 0.0f;
        [ForceUnroll]
        for (uint i = 0; i < N; i++)
            sum += v[int(i)];
        return sum;
    }
}

struct B : IInterface
{
    [Differentiable]
    static float calc<let N : uint>(vector<float, N> v)
    {
        float sum = 0.0f;
        [ForceUnroll]
        for (uint i = 0; i < N; i++)
            sum += 2.0f * v[int(i)];
        return sum;
    }
}

[Differentiable]
float run(IInterface obj, vector<float, 4> v)
{
    return obj.calc(v);
}

//TEST_INPUT: type_conformance A:IInterface = 0
//TEST_INPUT: type_conformance B:IInterface = 1

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    var dp = diffPair(
        vector<float, 4>(1.0f, 2.0f, 3.0f, 4.0f),
        vector<float, 4>(0.0f, 0.0f, 0.0f, 0.0f));

    var obj = createDynamicObject<IInterface>(dispatchThreadID.x, 0);
    bwd_diff(run)(obj, dp, 1.0f);
    outputBuffer[0] = dp.d[0]; // CHECK: 1.0
    outputBuffer[1] = dp.d[1]; // CHECK: 1.0

    dp = diffPair(
        vector<float, 4>(1.0f, 2.0f, 3.0f, 4.0f),
        vector<float, 4>(0.0f, 0.0f, 0.0f, 0.0f));
    obj = createDynamicObject<IInterface>(dispatchThreadID.x + 1, 0);
    bwd_diff(run)(obj, dp, 1.0f);
    outputBuffer[2] = dp.d[0]; // CHECK: 2.0
    outputBuffer[3] = dp.d[1]; // CHECK: 2.0
}
