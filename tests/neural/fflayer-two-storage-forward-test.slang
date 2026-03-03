// Test FFLayer eval() with separate weight/bias address instances.
//
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_TENSORVIEW=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-dx12 -compute -shaderobj -profile cs_6_6 -xslang -experimental-feature -output-using-type -xslang -DTEST_TENSORVIEW=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_TENSORVIEW=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature -xslang -DTEST_TENSORVIEW=0
//
// Compile-only test for TorchTensorViewAddress (CUDA target):
//TEST:SIMPLE(filecheck=CHECK): -target ptx -stage compute -entry computeMain -capability cuda_sm_7_0 -DTEST_TENSORVIEW=1 -experimental-feature
// CHECK: computeMain

import neural;

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

#if TEST_TENSORVIEW
uniform TensorView<float> weights;
uniform TensorView<float> bias;
typealias Address = TorchTensorViewAddress<float>;
#else
// Weights (Output=1, Input=2): [w0, w1]
//TEST_INPUT: set weights = ubuffer(data=[3.0 4.0], stride=4)
uniform RWStructuredBuffer<float>.Handle weights;
// Bias (Output=1): [b]
//TEST_INPUT: set bias = ubuffer(data=[0.5], stride=4)
uniform RWStructuredBuffer<float>.Handle bias;
typealias Address = BindlessAddress<float>;
#endif
typealias Vec2 = InlineVector<float, 2>;
typealias Vec1 = InlineVector<float, 1>;
typealias Act = IdentityActivation<float>;
typealias Layer = FFLayer<float, Vec2, Vec1, LinearLayout, Act, true>;

bool approxEqual(float a, float b, float eps = 0.001)
{
    return abs(a - b) < eps;
}

bool testTwoStorageForward()
{
    let weightAddr = Address(weights);
    let biasAddr = Address(bias);

    let layer = Layer();

    float[2] xArr = { 1.0, 2.0 };
    let x = Vec2(xArr);

    let y = layer.eval<Address>(x, weightAddr, biasAddr);

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

