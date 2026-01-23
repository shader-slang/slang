// Test FFLayer eval() with separate weight/bias storage instances.
//
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-dx12 -compute -shaderobj -profile cs_6_6 -xslang -experimental-feature -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -output-using-type -xslang -experimental-feature
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature

import neural;

// Weights (Output=1, Input=2): [w0, w1]
//TEST_INPUT: ubuffer(data=[3.0 4.0], stride=4):name=weights
RWStructuredBuffer<float> weights;

// Bias (Output=1): [b]
//TEST_INPUT: ubuffer(data=[0.5], stride=4):name=bias
RWStructuredBuffer<float> bias;

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

typealias Storage = StructuredBufferStorage<float>;
typealias Vec2 = InlineVector<float, 2>;
typealias Vec1 = InlineVector<float, 1>;
typealias Act = IdentityActivation<float>;
typealias Layer = FFLayer<float, Vec2, Vec1, Storage, Act, true>;

bool approxEqual(float a, float b, float eps = 0.001)
{
    return abs(a - b) < eps;
}

bool testTwoStorageForward()
{
    let wStorage = Storage(weights);
    let bStorage = Storage(bias);

    // weightAddress=0 in weights buffer, biasAddress=0 in bias buffer
    let layer = Layer(0, 0);

    float[2] xArr = { 1.0, 2.0 };
    let x = Vec2(xArr);

    let y = layer.eval<Storage>(wStorage, bStorage, x);

    // Expected: 3*1 + 4*2 + 0.5 = 11.5
    return approxEqual(y[0], 11.5);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    testResult[0] = testTwoStorageForward();
    // BUFFER: 1
}

